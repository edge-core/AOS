// header2xml.cpp
//
// Created by yehjunying on 12/09/18.
//
// Note:
// This file only build with llvm & clang at version 3.1.
//
// Usage:
// Read a C file and export all marco.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Host.h"

#include "llvm/ADT/SmallString.h"

#include "clang/AST/APValue.h"
#include "clang/Lex/LiteralSupport.h"

#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "clang/Basic/LangOptions.h"
#include "clang/Basic/FileSystemOptions.h"

#include "clang/Basic/SourceManager.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Basic/FileManager.h"

#include "clang/Frontend/HeaderSearchOptions.h"
#include "clang/Frontend/Utils.h"

#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"

#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/PreprocessorOptions.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Frontend/CompilerInstance.h"

typedef std::vector<std::string> vector_string;
typedef std::vector<std::string>::iterator vector_string_iterator;

class CommandLineArguments {
public:
    typedef enum {
        OutputFileFormatPlist,
        OutputFileFormatJSON,
        OutputFileFormatXML,
        OutputFileFormatDTD
    } OutputFileFormat;

    vector_string headerSearchPaths;
    vector_string sourceFiles;
    vector_string marcoExportInclude;
    vector_string marcoExportExclude;

    std::string outputFile;
    OutputFileFormat outputFileFormat;

    bool debugEnabled;

    CommandLineArguments() :
        outputFile("out.plist"){
        outputFileFormat = OutputFileFormatPlist;
        debugEnabled = false;
    }
};

class PPValue {
public:
    typedef enum {
        Uninitialized,
        Int,
        Bool,
        String
    } ValueKind;

private:
    ValueKind Kind;
    bool BoolVal;
    llvm::APSInt Val;
    std::string StringVal;

public:
    PPValue() {
        Kind = Uninitialized;
    }

    void setString(const std::string &S) {Kind = String; StringVal = S;}
    void setString(const llvm::StringRef &S) {this->setString(S.str());}

    void setInt(llvm::APSInt &I) {Kind = Int; Val = I;}
    void setBool(bool B) {Kind = Bool; BoolVal = B;}

    ValueKind getKind() const { return Kind; }

    bool getBool() const {
        assert(Kind == Bool);
        return BoolVal;
    }

    llvm::APSInt &getInt() {
        assert(Kind == Int);
        return Val;
    }

    const llvm::APSInt &getInt() const {
        assert(Kind == Int);
        return this->getInt();
    }

    std::string &getString() {
        assert(Kind == String);
        return StringVal;
    }

    const std::string &getString() const {
        assert(Kind == String);
        return this->getString();
    }

    std::string kindStr() const {
        switch (Kind) {
            case Int:           return "integer";
            case Bool:          return "bool";
            case String:        return "string";
            default:            return "undefined";
        }
    }

    bool isInteger() const {
        switch (Kind) {
            case Int:
            case Bool:          return true;
            default:            return false;
        }
    }

    int64_t toInteger() const {
        switch (Kind) {
            case Int:           return Val.getZExtValue();
            case Bool:          return (BoolVal) ? 1 : 0;
            default:            return 0;
        }
    }

    std::string str() const {
        std::stringstream out(std::stringstream::in | std::stringstream::out);

        switch (Kind) {
            case Int:
            case Bool:          out << this->toInteger(); break;
            case String:        out << StringVal; break;
            default:            out << "undefined"; break;
        }

        return out.str();
    }
};

static bool ParseCommandLine(int argc, const char *argv[], CommandLineArguments &out);

static void DoPrintMacros(clang::Preprocessor &PP, std::fstream &OS);

static void PrintMacroDefinition(const clang::IdentifierInfo &II, const clang::MacroInfo &MI,
                                 clang::Preprocessor &PP);

static bool
EvaluateExpression(PPValue &Result, const clang::IdentifierInfo &II,
                   const clang::MacroInfo &MI, clang::Preprocessor &PP);

static std::string XMLEscape(std::string string);

CommandLineArguments Arg;

int main(int argc, const char *argv[])
{
    ParseCommandLine(argc, argv, Arg);

    clang::DiagnosticOptions diagnosticOptions;
    clang::TextDiagnosticPrinter *pTextDiagnosticPrinter =
        new clang::TextDiagnosticPrinter(
            llvm::outs(),
            diagnosticOptions);
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;
    clang::DiagnosticsEngine *pDiagnosticsEngine =
        new clang::DiagnosticsEngine(pDiagIDs, pTextDiagnosticPrinter);

    clang::LangOptions languageOptions;
    clang::FileSystemOptions fileSystemOptions;
    clang::FileManager fileManager(fileSystemOptions);

    clang::SourceManager sourceManager(
        *pDiagnosticsEngine,
        fileManager);

    clang::HeaderSearchOptions headerSearchOptions;

    for (vector_string_iterator I = Arg.headerSearchPaths.begin(),
         E = Arg.headerSearchPaths.end();
         I != E; ++I) {
        headerSearchOptions.AddPath(*I,
            clang::frontend::Angled,
            false,
            false,
            false);
    }

    clang::TargetOptions targetOptions;
    targetOptions.Triple = llvm::sys::getDefaultTargetTriple();
    clang::TargetInfo *pTargetInfo =
        clang::TargetInfo::CreateTargetInfo(
            *pDiagnosticsEngine,
            targetOptions);

    clang::HeaderSearch headerSearch(fileManager,
                                     *pDiagnosticsEngine,
                                     languageOptions,
                                     pTargetInfo);
    clang::CompilerInstance compInst;

    clang::Preprocessor preprocessor(
        *pDiagnosticsEngine,
        languageOptions,
        pTargetInfo,
        sourceManager,
        headerSearch,
        compInst);

    clang::PreprocessorOptions preprocessorOptions;
    // disable predefined Macros so that you only see the tokens from your
    // source file. Note, this has some nasty side-effects like also undefning
    // your archictecture and things like that.
    //preprocessorOptions.UsePredefines = false;

    clang::FrontendOptions frontendOptions;
    clang::InitializePreprocessor(
        preprocessor,
        preprocessorOptions,
        headerSearchOptions,
        frontendOptions);

    for (vector_string_iterator I = Arg.sourceFiles.begin(),
         E = Arg.sourceFiles.end();
         I != E; ++I) {
        const clang::FileEntry *pFile = fileManager.getFile(*I);
        if (!pFile) {
            std::cerr << *I << " : No such file" << std::endl;
            return 1;
        }

        sourceManager.createMainFileID(pFile);
    }

    preprocessor.EnterMainSourceFile();
    pTextDiagnosticPrinter->BeginSourceFile(languageOptions, &preprocessor);

    clang::Token token;
    do {
        preprocessor.Lex(token);
        if( pDiagnosticsEngine->hasErrorOccurred())
        {
            break;
        }
//        preprocessor.DumpToken(token);
//        std::cerr << std::endl;
    } while( token.isNot(clang::tok::eof));
    pTextDiagnosticPrinter->EndSourceFile();

    std::fstream OutputFile(Arg.outputFile.c_str(), std::ios::out | std::ios::trunc);

    DoPrintMacros(preprocessor, OutputFile);

    return 0;
}

static bool ParseCommandLine(int argc, const char *argv[], CommandLineArguments &out) {
    std::string Opt;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") ||
            !strcmp(argv[i], "-help") ||
            !strcmp(argv[i], "--h") ||
            !strcmp(argv[i], "--help") ||
            !strcmp(argv[i], "/h") ||
            !strcmp(argv[i], "/?")) {

            std::cout << "Usage: header2xml [options] file ..." << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -help      Display this information" << std::endl;
            std::cout << "  -I<dir>    Add to header search path" << std::endl;
            std::cout << "  -Mi<name>  Export this marco" << std::endl;
            std::cout << "               By default, export marco with the following prefix:" << std::endl;
            std::cout << "                 SYS_CPNT, SYS_ADPT, SYS_DFLT, SYS_HWCFG" << std::endl;
            std::cout << "  -Mx<name>  Don't export this marco" << std::endl;
            std::cout << "  -o <file>  Name output file" << std::endl;
            std::cout << "  -Of <fmt>  Format output file" << std::endl;
            std::cout << "               XML   - XML" << std::endl;
            std::cout << "               DTD   - Document Type Definition" << std::endl;
            std::cout << "               Plist - Property list (default)" << std::endl;
            std::cout << "               JSON  - JavaScript Object Notation" << std::endl;
            std::cout << "  -debug     Enable debug" << std::endl;

            exit(0);
        }
    }

    for (int i = 1; i < argc; ++i) {
        if (!strncmp(argv[i], "-I", sizeof("-I")-1)) {
            Opt = std::string(argv[i]).substr(sizeof("-I")-1);
            out.headerSearchPaths.push_back(Opt);
        }
        else if (!strncmp(argv[i], "-Mi", sizeof("-Mi")-1)) {
            Opt = std::string(argv[i]).substr(sizeof("-Mi")-1);
            out.marcoExportInclude.push_back(Opt);
        }
        else if (!strncmp(argv[i], "-Mx", sizeof("-Mx")-1)) {
            Opt = std::string(argv[i]).substr(sizeof("-Mx")-1);
            out.marcoExportExclude.push_back(Opt);
        }
        else if (!strcmp(argv[i], "-o")) {
            if (argc <= ++i) {
                std::cerr << "argument to '-o' is missing";
                exit(1);
            }

            Opt = argv[i];
            out.outputFile = Opt;
        }
        else if ((!strcmp(argv[i], "-Of")) || (!strcmp(argv[i], "-output-file-format"))) {
            if (argc <= ++i) {
                std::cerr << "argument to '-Of' is missing";
                exit(1);
            }

#ifdef _POSIX_VERSION
#define stricmp strcasecmp
#endif

            if (!stricmp(argv[i], "XML")) {
                out.outputFileFormat = CommandLineArguments::OutputFileFormatXML;
            }
            else if (!stricmp(argv[i], "Plist")) {
                out.outputFileFormat = CommandLineArguments::OutputFileFormatPlist;
            }
            else if (!stricmp(argv[i], "JSON")) {
                out.outputFileFormat = CommandLineArguments::OutputFileFormatJSON;
            }
            else if (!stricmp(argv[i], "DTD")) {
                out.outputFileFormat = CommandLineArguments::OutputFileFormatDTD;
            }
            else {
                std::cerr << "Unsupported output file format";
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-debug")) {
            out.debugEnabled = true;
        }
        else {
            Opt = argv[i];
            out.sourceFiles.push_back(Opt);
        }
    }

    if (out.sourceFiles.size() == 0) {
        std::cerr << "no input files" << std::endl;
        exit(1);
    }

    return true;
}

typedef std::pair<clang::IdentifierInfo*, clang::MacroInfo*> id_macro_pair;
static int MacroIDCompare(const void* a, const void* b) {
    const id_macro_pair *LHS = static_cast<const id_macro_pair*>(a);
    const id_macro_pair *RHS = static_cast<const id_macro_pair*>(b);
    return LHS->first->getName().compare(RHS->first->getName());
}

template<class IteratorTy>
static inline void array_pod_sort(IteratorTy Start, IteratorTy End,
                                  int (*Compare)(const void*, const void*)) {
    // Don't dereference start iterator of empty sequence.
    if (Start == End) return;
    qsort(&*Start, End-Start, sizeof(*Start), Compare);
}

static bool SkipThisMarco(clang::IdentifierInfo &II) {
    for (vector_string_iterator I = Arg.marcoExportExclude.begin(),
         E = Arg.marcoExportExclude.end();
         I != E; ++I) {
        if (II.getName() == *I) {
            return true;
        }
    }

    for (vector_string_iterator I = Arg.marcoExportInclude.begin(),
         E = Arg.marcoExportInclude.end();
         I != E; ++I) {
        if (II.getName() == *I) {
            return false;
        }
    }

    if ((II.getName().substr(0, sizeof("SYS_CPNT_")-1) == "SYS_CPNT_") ||
        (II.getName().substr(0, sizeof("SYS_ADPT_")-1) == "SYS_ADPT_") ||
        (II.getName().substr(0, sizeof("SYS_DFLT_")-1) == "SYS_DFLT_") ||
        (II.getName().substr(0, sizeof("SYS_HWCFG_")-1) == "SYS_HWCFG_") ||
        (II.getName().substr(0, sizeof("MAX_")-1) == "MAX_") ||
        (II.getName().substr(0, sizeof("MIN_")-1) == "MIN_") ||
        (II.getName().substr(0, sizeof("MAXSIZE_")-1) == "MAXSIZE_") ||
        (II.getName().substr(0, sizeof("MINSIZE_")-1) == "MINSIZE_") ||
        (II.getName().substr(0, sizeof("EH_UI_")-1) == "EH_UI_")) {
        return false;
    }

    return true;
}

static void DoPrintMacros(clang::Preprocessor &PP, std::fstream &OS) {
    llvm::SmallVector<id_macro_pair, 128>
        MacrosByID(PP.macro_begin(), PP.macro_end());
    array_pod_sort(MacrosByID.begin(), MacrosByID.end(), MacroIDCompare);

    // FIXME: Change to Object
    switch (Arg.outputFileFormat) {
    case CommandLineArguments::OutputFileFormatPlist:
        OS << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        OS << "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << std::endl;
        OS << "<plist version=\"1.0\">" << std::endl;
        OS << "  <dict>" << std::endl;
        break;

    case CommandLineArguments::OutputFileFormatXML:
        OS << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        OS << "<cpnts>" << std::endl;
        break;

    case CommandLineArguments::OutputFileFormatDTD:
        OS << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        break;

    case CommandLineArguments::OutputFileFormatJSON:
        OS << "{" << std::endl;
        break;
    }

    for (unsigned i = 0, e = MacrosByID.size(); i != e; ++i) {
        clang::IdentifierInfo &II = *MacrosByID[i].first;
        clang::MacroInfo &MI = *MacrosByID[i].second;

        // Ignore computed macros like __LINE__ and friends.
        if (MI.isBuiltinMacro()) continue;

        if (!MI.getNumTokens()) continue;

        if (SkipThisMarco(II)) continue;

        if (Arg.debugEnabled) {
            PrintMacroDefinition(II, MI, PP);
        }

        // Not suport
        //
        // #define SYS_ADPT_QINQ_SERVICE_TAG_INFO_SRV_ACTION \
        //    ( SYS_VAL_vlanDot1qTunnelSrvAction_assignSvid )
        // #define SYS_VAL_vlanDot1qTunnelSrvAction_assignSvid     L_CVRT_SNMP_BIT_VALUE_32(2)
        //
        //

//        if (II.getName() != "SYS_ADPT_LPORT_IF_DESC_STR") {
//            continue;
//        }

        PPValue ResVal;

        EvaluateExpression(ResVal, II, MI, PP);

        /*
        //
        // Unit test section ...
        // FIXME: Move OUT here !!!!
        //
        //
        if (II.getName() == "SYS_ADPT_ALLOW_TO_BE_TM_PORT_LIST") {
            assert(ResVal.getKind() == PPValue::String);
            assert(ResVal.getString() == "0xFF,0xFF,0xFF,0xF0,");
            assert(ResVal.str() == "0xFF,0xFF,0xFF,0xF0,");
        }

        if (II.getName() == "SYS_ADPT_LPORT_IF_DESC_STR") {
            assert(ResVal.getKind() == PPValue::String);
            assert(ResVal.getString() == "Ethernet Port on unit $u%d, port $p%d");
            assert(ResVal.str() == "Ethernet Port on unit $u%d, port $p%d");
        }

//        if (II.getName() == "SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID0") {
//            assert(ResVal.getKind() == PPValue::String);
//            assert(ResVal.getString() == "ECS4510-28T");
//            assert(ResVal.str() == "ECS4510-28T");
//        }

        if (II.getName() == "SYS_ADPT_HTTP_WEB_LOGIN_PAGE_PATH") {
            assert(ResVal.getKind() == PPValue::String);
            assert(ResVal.str() == "/home/login.htm");
        }

        if (II.getName() == "SYS_ADPT_MIN_OSPF_EXT_LSDB_LIMIT") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.toInteger() == -1);
        }

        if (II.getName() == "SYS_CPNT_STORM_MODE") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 7);
        }

        if (II.getName() == "SYS_CPNT_MVR_SUPPORT_MULTI_DOMAIN_COMPATIBLE_SINGLE_DOMAIN") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 1);
        }

        if (II.getName() == "SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 2);
        }

        if (II.getName() == "SYS_CPNT_ACL_IPV6_EXT_SRC_IP_ADDR") {
            assert(ResVal.getKind() == PPValue::Bool);
            assert(ResVal.toInteger() == 1);
        }

        if (II.getName() == "SYS_ADPT_RADIUS_USE_SERVICE_TYPE_AS_PRIVILEGE") {
            assert(ResVal.getKind() == PPValue::Bool);
            assert(ResVal.toInteger() == 1);
        }

        if (II.getName() == "SYS_ADPT_EXTRA_TASK_STACK_SIZE") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 16384);
        }

        if (II.getName() == "SYS_ADPT_LOOPBACK_IF_INDEX_BASE") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 746);
        }

        if (II.getName() == "SYS_ADPT_MAX_NBR_OF_LAN_PACKET_TX_BUFFER") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 1786);
        }

        if (II.getName() == "SYS_ADPT_MAX_NBR_OF_MREF_DESC_BLOCK") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 1822);
        }

        if (II.getName() == "SYS_ADPT_DESTINATION_PORT_UNKNOWN") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 65535);
        }

        if (II.getName() == "SYS_ADPT_MAX_SNMP_TARGET_ADDR_TIMEOUT") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 2147483647);
        }

        if (II.getName() == "SYS_ADPT_LLDP_MED_CAPABILITY") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 39);
        }

        if (II.getName() == "SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST") {
            assert(ResVal.getKind() == PPValue::Int);
            assert(ResVal.getInt().getZExtValue() == 9);
        }
        */

        if (ResVal.getKind() == PPValue::Uninitialized) continue;

#ifdef _DEBUG
        //std::cout << II.getName().str() << "=" << ResVal.str() << " " << std::endl;
#endif // _DEBUG

        // FIXME: Change to Object
        switch (Arg.outputFileFormat) {
        case CommandLineArguments::OutputFileFormatPlist:
            OS << "    <key>" << II.getName().str() << "</key>" << std::endl;

            if (ResVal.getKind() == PPValue::Bool &&
                ResVal.getBool()) {
                OS << "    <true/>" << std::endl;
            }
            else if (ResVal.getKind() == PPValue::Bool &&
                     !ResVal.getBool()) {
                OS << "    <false/>" << std::endl;
            }
            else {

                //
                // I dont know what's going on this constant !!
                // #define SYS_DFLT_RIP_IF_CONF_AUTH_KEY           "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                // So convert to C string again.
                //

                OS << "    <" << ResVal.kindStr() << ">"
                   << ResVal.str().c_str()
                   << "</" << ResVal.kindStr() << ">" << std::endl;
            }
            break;

        case CommandLineArguments::OutputFileFormatXML:
            OS << "  <cpnt name=\"" << II.getName().str() << "\"" << std::endl;
            OS << "        type=\"" << ResVal.kindStr() << "\"" << std::endl;

            if (ResVal.getKind() == PPValue::Bool &&
                ResVal.getBool()) {
                OS << "        value=\"true\"/>" << std::endl;
            }
            else if (ResVal.getKind() == PPValue::Bool &&
                     !ResVal.getBool()) {
                OS << "        value=\"false\"/>" << std::endl;
            }
            else {
                OS << "        value=\"" << XMLEscape(ResVal.str()).c_str() << "\"/>" << std::endl;
            }
            break;

        case CommandLineArguments::OutputFileFormatDTD:
            OS << "<!ENTITY " << II.getName().str() << " ";
            OS << "\"";

            if (ResVal.getKind() == PPValue::Bool &&
                ResVal.getBool()) {
                OS << "1";
            }
            else if (ResVal.getKind() == PPValue::Bool &&
                     !ResVal.getBool()) {
                OS << "0";
            }
            else {
                OS << XMLEscape(ResVal.str()).c_str();
            }

            OS << "\"";
            OS << ">" << std::endl;
            break;

        case CommandLineArguments::OutputFileFormatJSON:
            OS << "  \"" << II.getName().str() << "\" : ";

            if (ResVal.getKind() == PPValue::Bool &&
                ResVal.getBool()) {
                OS << "true";
            }
            else if (ResVal.getKind() == PPValue::Bool &&
                     !ResVal.getBool()) {
                OS << "false";
            }
            else if (ResVal.getKind() == PPValue::Int) {
                OS << ResVal.str();
            }
            else if (ResVal.getKind() == PPValue::String) {
                OS << "\"" << ResVal.str().c_str() << "\"";
            }

            OS << "," << std::endl;

            break;
        }
    }

    // FIXME: Change to Object
    switch (Arg.outputFileFormat) {
    case CommandLineArguments::OutputFileFormatPlist:
        OS << "  </dict>" << std::endl;
        OS << "</plist>" << std::endl;
        break;

    case CommandLineArguments::OutputFileFormatXML:
        OS << "</cpnts>" << std::endl;
        break;

    case CommandLineArguments::OutputFileFormatDTD:
        break;

    case CommandLineArguments::OutputFileFormatJSON:
        //OS << "  }" << std::endl;
        OS << "}" << std::endl;
        break;
    }
}

static void PrintMacroDefinition(const clang::IdentifierInfo &II, const clang::MacroInfo &MI,
                                 clang::Preprocessor &PP) {
    std::cout << "#define " << II.getName().str();

    if (MI.isFunctionLike()) {
        std::cout << '(';

        if (!MI.arg_empty()) {
            clang::MacroInfo::arg_iterator AI = MI.arg_begin(), E = MI.arg_end();
        for (; AI+1 != E; ++AI) {
            std::cout << (*AI)->getName().str();
            std::cout << ',';
        }

        // Last argument.
        if ((*AI)->getName() == "__VA_ARGS__")
            std::cout << "...";
        else
            std::cout << (*AI)->getName().str();
        }

        if (MI.isGNUVarargs())
            std::cout << "...";  // #define foo(x...)

        std::cout << ')';
    }

    // GCC always emits a space, even if the macro body is empty.  However, do not
    // want to emit two spaces if the first token has a leading space.
    if (MI.tokens_empty() || !MI.tokens_begin()->hasLeadingSpace())
        std::cerr << ' ';

    for (clang::MacroInfo::tokens_iterator I = MI.tokens_begin(), E = MI.tokens_end();
        I != E; ++I) {
        if (I->hasLeadingSpace())
            std::cout << ' ';

        std::string SpellingBuffer = PP.getSpelling(*I);
        std::cout << SpellingBuffer;
    }

    std::cout << std::endl;
}

void
PackAllToStringValue(PPValue &Result, const clang::IdentifierInfo &II,
                     const clang::MacroInfo &MI, clang::Preprocessor &PP) {
    std::stringstream N(std::stringstream::in | std::stringstream::out);

    for (clang::MacroInfo::tokens_iterator I = MI.tokens_begin(), E = MI.tokens_end();
        I != E; ++I) {
        std::string SpellingBuffer = PP.getSpelling(*I);
        N << SpellingBuffer;
    }

    Result.setString(N.str());
}

/// getPrecedence - Return the precedence of the specified binary operator
/// token.  This returns:
///   ~0 - Invalid token.
///   14 -> 3 - various operators.
///    0 - 'eod' or ')'
static unsigned getPrecedence(clang::tok::TokenKind Kind) {
    using namespace clang;

    switch (Kind) {
    default: return ~0U;
    case tok::identifier:
    case tok::numeric_constant:     return 16;
    case tok::exclaim:              return 15;
    case tok::percent:
    case tok::slash:
    case tok::star:                 return 14;
    case tok::plus:
    case tok::minus:                return 13;
    case tok::lessless:
    case tok::greatergreater:       return 12;
    case tok::lessequal:
    case tok::less:
    case tok::greaterequal:
    case tok::greater:              return 11;
    case tok::exclaimequal:
    case tok::equalequal:           return 10;
    case tok::amp:                  return 9;
    case tok::caret:                return 8;
    case tok::pipe:                 return 7;
    case tok::ampamp:               return 6;
    case tok::pipepipe:             return 5;
    case tok::question:             return 4;
    case tok::comma:                return 3;
    case tok::colon:                return 2;
    case tok::r_paren:              return 0;// Lowest priority, end of expr.
    case tok::eod:                  return 0;// Lowest priority, end of directive.
    }
}

static bool
EvaluateExpression(PPValue &Result, const clang::IdentifierInfo &II,
                   const clang::MacroInfo &MI, clang::Preprocessor &PP) {

    // BAD BAD BAD
    //
    // Patch for
    // #define SYS_ADPT_PRIVATEMIB_OID                             1, 3, 6, 1, 4, 1, 259, 10, 1, 24, 1
    //
    // #define SYS_ADPT_ALLOW_TO_BE_TM_PORT_LIST \
	//		0xFF,   /*0b11111111*//* port 1-8	*/  \
	//		0xFF,   /*0b11111111*//* port 9-16  */  \
	//		0xFF,	/*0b11111111*//* port 17-24 */  \
	//		0xF0,	/*0b11110000*//* port 25-28 */
    //
    for (clang::MacroInfo::tokens_iterator I = MI.tokens_begin(), E = MI.tokens_end();
        I != E; ++I) {
        if (I->is(clang::tok::comma)) {
            PackAllToStringValue(Result, II, MI, PP);
            return true;
        }
    }

    std::vector<clang::Token> Stack;
    std::vector<clang::Token> Polish;

    for (clang::MacroInfo::tokens_iterator I = MI.tokens_begin(), E = MI.tokens_end();
        I != E; ++I) {
//        PP.DumpToken(*I);

        unsigned ThisPrec = getPrecedence(I->getKind());

        while (!Stack.empty() && ThisPrec <= getPrecedence(Stack.back().getKind()) && Stack.back().getKind() != clang::tok::l_paren) {
            Polish.push_back(Stack.back());
            Stack.pop_back();
        }

        if (I->getKind() != clang::tok::r_paren) {
            Stack.push_back(*I);
        }
        else {
            if (!Stack.empty()) {
                Stack.pop_back();
            }
        }
    }

    while (!Stack.empty()) {
        Polish.push_back(Stack.back());
        Stack.pop_back();
    }

    if (Arg.debugEnabled) {
        std::cerr << '\n';
        std::cerr << "Reverse Polish Notation: ";

        for (std::vector<clang::Token>::iterator I = Polish.begin(), E = Polish.end();
            I != E; ++I) {
            std::cerr << PP.getSpelling(*I) << ' ';
        }

        std::cerr << std::endl;
        std::cerr << std::endl;
    }

    // Evaluate Value
    std::vector<PPValue> EvaluateStack;

    for (std::vector<clang::Token>::iterator I = Polish.begin(), E = Polish.end();
        I != E; ++I) {
//        PP.DumpToken(*I);

        const clang::Token &PeekTok = *I;
        clang::IdentifierInfo *ThisII;

        switch (I->getKind()) {
        case clang::tok::identifier: {
            ThisII = I->getIdentifierInfo();

            if (ThisII->hasMacroDefinition()) {
                clang::MacroInfo *ThisMI = PP.getMacroInfo(ThisII);

                PPValue ResVal;

                if (true == EvaluateExpression(ResVal, *ThisII, *ThisMI, PP)) {
                    return true;
                }

                EvaluateStack.push_back(ResVal);

                assert(EvaluateStack.back().getKind() == ResVal.getKind());

                break;
            }

            return true;
        }

        case clang::tok::numeric_constant: {
            llvm::SmallString<64> IntegerBuffer;
            bool NumberInvalid = false;

            llvm::StringRef Spelling = PP.getSpelling(PeekTok, IntegerBuffer,
                                                      &NumberInvalid);
            if (NumberInvalid)
                //return true; // a diagnostic was already reported
                break;

            clang::NumericLiteralParser Literal(Spelling.begin(), Spelling.end(),
                                                PeekTok.getLocation(), PP);
            if (Literal.hadError)
                return true; // a diagnostic was already reported.

            if (Literal.isFloatingLiteral() || Literal.isImaginary) {
              //PP.Diag(PeekTok, diag::err_pp_illegal_floating_literal);
                return true;
            }

            unsigned BitWidth = PP.getTargetInfo().getIntMaxTWidth();
            llvm::APSInt N(BitWidth);

            // Parse the integer literal into Result.
            if (Literal.GetIntegerValue(/*Result.Val*/ N)) {
//                    // Overflow parsing integer literal.
//                    if (ValueLive) PP.Diag(PeekTok, diag::warn_integer_too_large);
                /*Result.Val*/N.setIsUnsigned(true);
            } else {
                // Set the signedness of the result to match whether there was a U suffix
                // or not.
                /*Result.Val*/N.setIsUnsigned(Literal.isUnsigned);

                // Detect overflow based on whether the value is signed.  If signed
                // and if the value is too large, emit a warning "integer constant is so
                // large that it is unsigned" e.g. on 12345678901234567890 where intmax_t
                // is 64-bits.
                if (!Literal.isUnsigned && /*Result.Val*/N.isNegative()) {
                    // Don't warn for a hex literal: 0x8000..0 shouldn't warn.
//                        if (ValueLive && Literal.getRadix() != 16)
//                            PP.Diag(PeekTok, diag::warn_integer_too_large_for_signed);
                    /*Result.Val*/N.setIsUnsigned(true);
                }
            }

            Result.setInt(N);

            EvaluateStack.push_back(Result);

            assert(EvaluateStack.back().getKind() == PPValue::Int);

            break;
        }

        case clang::tok::string_literal: {
            llvm::SmallString<64> IntegerBuffer;
            bool NumberInvalid = false;
            llvm::StringRef Spelling = PP.getSpelling(PeekTok, IntegerBuffer);
            //if (NumberInvalid)
            //    //return true; // a diagnostic was already reported
            //    break;

            clang::StringLiteralParser Literal(&PeekTok, 1, PP);
            if (Literal.hadError)
                return true; // a diagnostic was already reported.

            llvm::StringRef N = Literal.GetString();

            Result.setString(N);

            EvaluateStack.push_back(Result);

            assert(EvaluateStack.back().getKind() == PPValue::String);

            break;
        }

        case clang::tok::star:
        case clang::tok::slash:
        case clang::tok::percent:
        case clang::tok::plus:
        case clang::tok::minus:
        case clang::tok::amp:
        case clang::tok::pipe:
        case clang::tok::lessless:
        case clang::tok::greatergreater: {
            PPValue N2 = EvaluateStack.back();

            if (!N2.isInteger()) {
                return true;
            }

            EvaluateStack.pop_back();

            PPValue N1;

            //
            // Patch for (-1)
            //
            if (!EvaluateStack.empty()) {
                N1 = EvaluateStack.back();
                if (!N1.isInteger()) {
                    return true;
                }
            }
            else {
                EvaluateStack.push_back(N2);
            }

            int64_t n1 = N1.toInteger();
            int64_t n2 = N2.toInteger();

            unsigned BitWidth = PP.getTargetInfo().getIntMaxTWidth();
            llvm::APSInt N(BitWidth);

            if (I->getKind() == clang::tok::star) {
                N = uint64_t(n1 * n2);
            }
            else if (I->getKind() == clang::tok::slash) {
                N = uint64_t(n1 / n2);
            }
            else if (I->getKind() == clang::tok::percent) {
                N = uint64_t(n1 % n2);
            }
            else if (I->getKind() == clang::tok::plus) {
                N = uint64_t(n1 + n2);
            }
            else if (I->getKind() == clang::tok::minus) {
                N = uint64_t(n1 - n2);
            }
            else if (I->getKind() == clang::tok::amp) {
                N = uint64_t(n1 & n2);
            }
            else if (I->getKind() == clang::tok::pipe) {
                N = uint64_t(n1 | n2);
            }
            else if (I->getKind() == clang::tok::lessless) {
                N = uint64_t(n1 << n2);
            }
            else if (I->getKind() == clang::tok::greatergreater) {
                N = uint64_t(n1 >> n2);
            }

            EvaluateStack.back().setInt(N);

            break;
        }

        case clang::tok::equalequal:
        case clang::tok::pipepipe:
        case clang::tok::ampamp: {
            PPValue N2 = EvaluateStack.back();

            if (!N2.isInteger()) {
                return true;
            }

            EvaluateStack.pop_back();

            PPValue N1 = EvaluateStack.back();

            if (!N1.isInteger()) {
                return true;
            }

            int64_t n1 = N1.toInteger();
            int64_t n2 = N2.toInteger();

            bool N;

            if (I->getKind() == clang::tok::equalequal) {
                N = (n1 == n2);
            }
            else if (I->getKind() == clang::tok::pipepipe) {
                N = (n1 || n2);
            }
            else if (I->getKind() == clang::tok::ampamp) {
                N = (n1 && n2);
            }

            EvaluateStack.back().setBool(N);

            break;
        }

        case clang::tok::exclaim: {
            PPValue N = EvaluateStack.back();

            if (N.getKind() != PPValue::Int &&
                N.getKind() != PPValue::Bool) {
                return true;
            }

            if (N.toInteger()) {
                N.setBool(false);
            }
            else {
                N.setBool(true);
            }

            EvaluateStack.back() = N;

            break;
        }

        default:
            break;
        }
    }

    if (II.getName() == "TRUE" ||
        II.getName() == "true") {
        Result.setBool(true);
    }
    else if (II.getName() == "FALSE" ||
             II.getName() == "false") {
        Result.setBool(false);
    }
    else {
        if (!EvaluateStack.empty()) {
            Result = EvaluateStack.back();
        }
    }

    return false;
}

static void myReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    size_t pos = 0;
    while((pos = str.find(oldStr, pos)) != std::string::npos) {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

static std::string XMLEscape(std::string string) {

    myReplace(string, "<", "&lt;");
    myReplace(string, ">", "&gt;");
    myReplace(string, "&", "&amp;");
    myReplace(string, "%", "&#37;");
    myReplace(string, "\"", "&quot;");
    myReplace(string, "\'", "&apos;");

    return string;
}
