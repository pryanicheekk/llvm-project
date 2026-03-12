// RUN: clang-18 -fplugin=%S/../ImplicitConversionsCounter.so -c %s 2>&1 | FileCheck %s

// CHECK-LABEL: Function `complex_function`
// CHECK: double -> int: 1
// CHECK: float -> double: 4
// CHECK: float[5] -> float *: 2
// CHECK: int -> double: 3
// CHECK: int -> float: 6

// CHECK-LABEL: Function `mul`
// CHECK: double -> int: 1
// CHECK: float -> double: 1
// CHECK: float -> int: 1

// CHECK-LABEL: Function `sum`
// CHECK: float -> double: 1
// CHECK: int -> float: 1

double sum(int a, float b) {
    return a + b;
}

int mul(float a, float b) {
    return a + sum(a, b);
}

double complex_function(int a, float b, double c, char d) {
    double r1 = a + b;
    double r2 = b + c;
    int r3 = a + d;
    
    if (a > b) {
        float x = a;
    } else {
        int y = c;
    }
    
    int r4 = (b > c) ? a : (int)c;
    
    float sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += i * b;
    }
    
    float arr[5];
    for (int i = 0; i < 5; i++) {
        arr[i] = i;
    }
    
    double total = 0;
    for (int i = 0; i < 5; i++) {
        total += arr[i];
    }
    
    return r1 + r2 + r3 + r4 + total;
}   