#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include <iostream>
#include <map>
#include <string>

using namespace clang;

namespace {

struct ConversionStats {
    std::map<std::string, int> conversions;
    
    void addConversion(const std::string& from, const std::string& to) {
        std::string key = from + " -> " + to;
        conversions[key]++;
    }
    
    void print(const std::string& funcName) const {
        std::cout << "Function `" << funcName << "`" << std::endl;
        for (const auto& conv : conversions) {
            std::cout << conv.first << ": " << conv.second << std::endl;
        }
        std::cout << std::endl;
    }
};

class ConversionVisitor : public RecursiveASTVisitor<ConversionVisitor> {
private:
    std::map<std::string, ConversionStats>& FunctionStats;
    std::string CurrentFunction;
    
    std::string getTypeName(QualType T) {
    T = T.getCanonicalType();
    
    // Если это указатель на функцию, возвращаем просто "function"
    if (T->isFunctionPointerType() || T->isFunctionType()) {
        return "function";
    }
    
    if (T->isIntegerType()) {
        return "int";
    }
    if (T->isFloatingType()) {
        if (T->isSpecificBuiltinType(BuiltinType::Float)) return "float";
        if (T->isSpecificBuiltinType(BuiltinType::Double)) return "double";
    }
    
    // Для всего остального возвращаем упрощенное имя
    std::string name = T.getAsString();
    // Если это сложный тип, берем только базовое имя
    size_t pos = name.find('(');
    if (pos != std::string::npos) {
        return "function";
    }
    return name;
}
    
public:
    ConversionVisitor(std::map<std::string, ConversionStats>& stats) 
        : FunctionStats(stats) {}
    
    bool VisitFunctionDecl(FunctionDecl *FD) {
        if (FD->hasBody()) {
            CurrentFunction = FD->getNameAsString();
        }
        return true;
    }
    
    bool VisitImplicitCastExpr(ImplicitCastExpr *ICE) {
        if (CurrentFunction.empty()) return true;
        
        // Пропускаем служебные преобразования
        CastKind kind = ICE->getCastKind();
        if (kind == CK_LValueToRValue || kind == CK_NoOp) {
            return true;
        }
        
        std::string fromType = getTypeName(ICE->getSubExpr()->getType());
        std::string toType = getTypeName(ICE->getType());
        
        if (fromType != toType) {
            FunctionStats[CurrentFunction].addConversion(fromType, toType);
        }
        
        return true;
    }
};

class ConversionCounterConsumer : public ASTConsumer {
public:
    void HandleTranslationUnit(ASTContext &Context) override {
        std::map<std::string, ConversionStats> functionStats;
        
        ConversionVisitor Visitor(functionStats);
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
        
        for (const auto& pair : functionStats) {
            pair.second.print(pair.first);
        }
    }
};

class ImplicitConversionsAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                   llvm::StringRef InFile) override {
        return std::make_unique<ConversionCounterConsumer>();
    }
    
    bool ParseArgs(const CompilerInstance &CI,
                   const std::vector<std::string> &args) override {
        return true;
    }
    
    // ВАЖНО: этот метод необходим для работы плагина
    ActionType getActionType() override {
        return AddBeforeMainAction;
    }
};

} // namespace

static FrontendPluginRegistry::Add<ImplicitConversionsAction>
X("implicit-conversions", "count implicit type conversions in functions");