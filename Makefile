CXX = clang++-18
LLVM_CONFIG = llvm-config-18

CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags) -fPIC
LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags) -shared
LIBS = $(shell $(LLVM_CONFIG) --libs) \
       -lclangAST \
       -lclangBasic \
       -lclangFrontend \
       -lclangSema

TARGET = ImplicitConversionsCounter.so

all: $(TARGET)

$(TARGET): ImplicitConversionsCounter.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET) *.o

test: $(TARGET)
	clang-18 -fplugin=./$(TARGET) -c tests/test.c

lit-test: $(TARGET)
	cd tests && PATH=/usr/lib/llvm-18/bin:$$PATH lit -v .

.PHONY: all clean test lit-test