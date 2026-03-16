CXX = clang++
CXXFLAGS = $(shell llvm-config --cxxflags) -fPIC
LDFLAGS = $(shell llvm-config --ldflags --libs core analysis) -shared

all: output_from_ll.ll output_from_cpp.cpp

LoopInstrument.so: LoopInstrument.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

output_from_ll.ll: LoopInstrument.so test.ll
	opt -load-pass-plugin ./LoopInstrument.so -passes=loop-instrument -S test.ll -o $@

output_from_cpp.cpp: LoopInstrument.so test.cpp
	clang++ -S -emit-llvm -O0 test.cpp -o test_from_cpp.ll
	opt -load-pass-plugin ./LoopInstrument.so -passes=loop-instrument -S test_from_cpp.ll -o output_from_cpp.ll
	mv output_from_cpp.ll $@

clean:
	rm -f LoopInstrument.so output_from_ll.ll output_from_cpp.cpp test_from_cpp.ll

.PHONY: all clean
