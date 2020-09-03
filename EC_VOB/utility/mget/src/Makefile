LLVM_SRC_ROOT  := ../build/llvm-3.1.src
LLVM_OBJ_ROOT  := ../build/llvm-3.1.build
LLVM_BIN_ROOT  := ../build/llvm-3.1.build/bin
CLANG_SRC_ROOT := $(LLVM_SRC_ROOT)/tools/clang
CLANG_OBJ_ROOT := $(LLVM_OBJ_ROOT)/tools/clang

LLVMCOMPONENTS := cppbackend
RTTIFLAG := -fno-rtti
LLVMCONFIG := $(LLVM_BIN_ROOT)/llvm-config

CXXFLAGS := -D_POSIX_VERSION \
            -I$(CLANG_SRC_ROOT)/include \
            -I$(CLANG_OBJ_ROOT)/include \
            $(shell $(LLVMCONFIG) --cxxflags) $(RTTIFLAG)
LLVMLDFLAGS := $(shell $(LLVMCONFIG) --ldflags --libs $(LLVMCOMPONENTS))

SOURCES = mget.cpp

CLANGLIBS = \
	-lclangTooling\
	-lclangFrontendTool\
	-lclangFrontend\
	-lclangDriver\
	-lclangSerialization\
	-lclangCodeGen\
	-lclangParse\
	-lclangSema\
	-lclangStaticAnalyzerFrontend\
	-lclangStaticAnalyzerCheckers\
	-lclangStaticAnalyzerCore\
	-lclangAnalysis\
	-lclangARCMigrate\
	-lclangEdit\
	-lclangAST\
	-lclangLex\
	-lclangBasic\
	-lLLVMSupport\
	-lLLVMMCDisassembler\
	-lLLVMMC\
	$(shell $(LLVMCONFIG) --libs)

OBJECTS = $(SOURCES:.cpp=.o)
EXES = $(OBJECTS:.o=)

all: $(OBJECTS) $(EXES)

%: %.o
	$(CXX) -o $@ $< $(CLANGLIBS) $(LLVMLDFLAGS)

clean:
	-rm -f $(EXES) $(OBJECTS) *~
