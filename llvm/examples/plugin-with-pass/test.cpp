// test.cpp - Исходный код для демонстрации работы пасса
#include <iostream>

// Функции, которые будут вставлены пассом
extern "C" void loop_start() {
    std::cout << "Loop started" << std::endl;
}

extern "C" void loop_end() {
    std::cout << "Loop ended" << std::endl;
}

// Функция с вложенными циклами
void nested_loops(int n) {
    std::cout << "=== nested_loops ===" << std::endl;
    
    // Внешний цикл
    for (int i = 0; i < n; i++) {
        // Внутренний цикл
        for (int j = 0; j < n; j++) {
            // Тело цикла
            int x = i + j;
        }
    }
}

// Функция с простым циклом
void simple_loop(int n) {
    std::cout << "=== simple_loop ===" << std::endl;
    
    // Простой цикл
    for (int i = 0; i < n; i++) {
        // Тело цикла
        int x = i * 2;
    }
}

// Функция с условным выходом
void loop_with_break(int n, int threshold) {
    std::cout << "=== loop_with_break ===" << std::endl;
    
    // Цикл с break
    for (int i = 0; i < n; i++) {
        if (i > threshold) {
            break;  // Ранний выход из цикла
        }
        int x = i * i;
    }
}

int main() {
    std::cout << "Testing loop instrumentation pass" << std::endl;
    std::cout << "=================================" << std::endl;
    
    nested_loops(3);
    simple_loop(4);
    loop_with_break(10, 5);
    
    return 0;
}