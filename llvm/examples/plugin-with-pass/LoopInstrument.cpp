#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "loop-instrument"

namespace {

/**
 * Вспомогательная функция для получения или создания внешней функции
 * @param M Указатель на модуль
 * @param Name Имя функции
 * @return FunctionCallee для вызова функции
 */
static FunctionCallee getOrInsertLoopFunction(Module *M, StringRef Name) {
  LLVMContext &Ctx = M->getContext();
  FunctionType *FuncTy = FunctionType::get(Type::getVoidTy(Ctx), false);
  return M->getOrInsertFunction(Name, FuncTy);
}

/**
 * Основной класс пасса для инструментирования циклов
 */
struct LoopInstrumentPass : public PassInfoMixin<LoopInstrumentPass> {
  
private:
  /**
   * Рекурсивная функция для обработки циклов и вложенных циклов
   * @param L Текущий цикл
   * @param F Функция, содержащая цикл
   * @param StartFunc Функция loop_start
   * @param EndFunc Функция loop_end
   * @param Builder IRBuilder для создания инструкций
   */
  void instrumentLoop(Loop *L, Function *F, FunctionCallee StartFunc, 
                      FunctionCallee EndFunc, IRBuilder<> &Builder) {
    
    // Статический счетчик для уникальных имен блоков
    static int BlockCounter = 0;
    
    // Сначала обрабатываем все вложенные циклы (обход в глубину)
    for (Loop *SubLoop : L->getSubLoops()) {
      instrumentLoop(SubLoop, F, StartFunc, EndFunc, Builder);
    }
    
    // Получаем pre-header цикла - блок, откуда всегда попадают в цикл
    BasicBlock *PreHeader = L->getLoopPreheader();
    if (!PreHeader) {
      errs() << "Warning: Loop in function " << F->getName() 
             << " has no preheader, skipping\n";
      return;
    }
    
    // Вставляем вызов loop_start() в конце pre-header
    Builder.SetInsertPoint(PreHeader->getTerminator());
    Builder.CreateCall(StartFunc);
    LLVM_DEBUG(dbgs() << "  Inserted loop_start() in preheader of loop\n");
    
    // Получаем все блоки, из которых можно выйти из цикла
    SmallVector<BasicBlock*, 8> ExitingBlocks;
    L->getExitingBlocks(ExitingBlocks);
    
    // Для каждого выхода создаем новый базовый блок с loop_end()
    for (BasicBlock *ExitingBlock : ExitingBlocks) {
      Instruction *Terminator = ExitingBlock->getTerminator();
      
      // Для каждого successor, который вне цикла, вставляем loop_end()
      for (unsigned i = 0; i < Terminator->getNumSuccessors(); i++) {
        BasicBlock *Succ = Terminator->getSuccessor(i);
        
        // Если successor вне цикла - это выход
        if (!L->contains(Succ)) {
          // Создаем уникальное имя для нового блока
          std::string BlockName = "loop_end." + std::to_string(++BlockCounter);
          
          // Создаем новый блок между ExitingBlock и Succ
          BasicBlock *NewBlock = BasicBlock::Create(F->getContext(), 
                                                    BlockName, 
                                                    F, 
                                                    Succ);
          
          // Вставляем loop_end() в новый блок
          Builder.SetInsertPoint(NewBlock);
          Builder.CreateCall(EndFunc);
          
          // Добавляем безусловный переход к Succ
          Builder.CreateBr(Succ);
          
          // Перенаправляем Terminator на новый блок
          Terminator->setSuccessor(i, NewBlock);
          
          LLVM_DEBUG(dbgs() << "  Inserted loop_end() block " << BlockName 
                            << " before exit to " << Succ->getName() << "\n");
        }
      }
    }
  }
  
public:
  /**
   * Основная функция запуска пасса
   * @param F Функция для обработки
   * @param FAM Менеджер анализов
   * @return Какие анализы сохранены
   */
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    // Получаем анализ циклов для текущей функции
    LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
    
    // Если нет циклов, ничего не делаем
    if (LI.empty()) {
      LLVM_DEBUG(dbgs() << "\nFunction " << F.getName() 
                        << ": no loops found\n");
      return PreservedAnalyses::all();
    }
    
    LLVM_DEBUG(dbgs() << "\nProcessing function: " << F.getName() << "\n"
                      << "  Found " << LI.getTopLevelLoops().size() 
                      << " top-level loops\n");
    
    // Получаем модуль для создания функций
    Module *M = F.getParent();
    
    // Получаем или создаем функции loop_start и loop_end
    FunctionCallee StartFunc = getOrInsertLoopFunction(M, "loop_start");
    FunctionCallee EndFunc = getOrInsertLoopFunction(M, "loop_end");
    
    // Настраиваем атрибуты функций
    if (auto *StartFn = dyn_cast<Function>(StartFunc.getCallee())) {
      StartFn->setLinkage(GlobalValue::ExternalLinkage);
      StartFn->addFnAttr(Attribute::NoUnwind);
    }
    if (auto *EndFn = dyn_cast<Function>(EndFunc.getCallee())) {
      EndFn->setLinkage(GlobalValue::ExternalLinkage);
      EndFn->addFnAttr(Attribute::NoUnwind);
    }
    
    // Создаем IRBuilder для генерации кода
    IRBuilder<> Builder(F.getContext());
    
    // Обрабатываем все циклы верхнего уровня
    for (Loop *L : LI) {
      instrumentLoop(L, &F, StartFunc, EndFunc, Builder);
    }
    
    // Сохраняем анализ циклов (мы его не изменяем)
    PreservedAnalyses PA;
    PA.preserve<LoopAnalysis>();
    return PA;
  }
};

} // anonymous namespace

//===----------------------------------------------------------------------===//
// Регистрация плагина для нового Pass Manager'а
//===----------------------------------------------------------------------===//

/**
 * Возвращает информацию о плагине для LLVM
 */
llvm::PassPluginLibraryInfo getLoopInstrumentPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LoopInstrument", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            // Регистрируем парсинг командной строки
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "loop-instrument") {
                    FPM.addPass(LoopInstrumentPass());
                    return true;
                  }
                  return false;
                });
          }};
}

/**
 * Точка входа для плагина (для динамической загрузки)
 */
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLoopInstrumentPluginInfo();
}