// RUN: clang-18 -c -fplugin=%S/../MaybeUnused.so %s 2>&1 | FileCheck %s --allow-empty
// RUN: clang-18 -Wunused-variable -Wunused-parameter -c %s 2>&1 | FileCheck %s --check-prefix=WITHOUT-PLUGIN
// RUN: clang-18 -Wunused-variable -Wunused-parameter -fplugin=%S/../MaybeUnused.so -c %s 2>&1 | FileCheck %s --check-prefix=WITH-PLUGIN

// WITHOUT-PLUGIN: warning: unused variable 'value'
// WITHOUT-PLUGIN: warning: unused parameter 'c'
// WITHOUT-PLUGIN: warning: unused variable 'unused1'
// WITHOUT-PLUGIN: warning: unused variable 'unused2'
// WITHOUT-PLUGIN: warning: unused parameter 'z'
// WITHOUT-PLUGIN: warning: unused parameter 'c' [[#]]
// WITHOUT-PLUGIN: warning: unused parameter 'd'
// WITHOUT-PLUGIN: warning: unused variable 'temp1'
// WITHOUT-PLUGIN: warning: unused variable 'temp2'
// WITHOUT-PLUGIN: warning: unused variable 'temp3'

// WITH-PLUGIN-NOT: warning: unused

int foo(int a, int b, int c) {
    double value = 0.0;
    return a + b;
}

int bar(int x, int y, int z) {
    int used = x + y;
    int unused1 = 42;
    int unused2 = 100;
    return used;
}

double baz(double a, float b, int c, char d) {
    double result = a + b;
    float temp1 = 3.14;
    int temp2 = 42;
    char temp3 = 'A';
    return result;
}