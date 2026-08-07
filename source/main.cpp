#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <nlohmann/json.hpp>

#include "SymbolActionFactory.hpp"
#include "SymbolConsumer.hpp"


static llvm::cl::OptionCategory tool_category("clang-architecture options");

static llvm::cl::opt<std::string> tool_option_output(
    "output",
    llvm::cl::cat(tool_category),
    llvm::cl::Optional,
    llvm::cl::desc("output table path"),
    llvm::cl::value_desc("path"),
    llvm::cl::init("-")
);
static llvm::cl::alias tool_option_output_short(
    "o",
    llvm::cl::cat(tool_category),
    llvm::cl::aliasopt(tool_option_output),
    llvm::cl::desc("Alias for --output")
);
static llvm::cl::opt<bool> tool_option_keep_system(
    "keep-system",
    llvm::cl::cat(tool_category),
    llvm::cl::desc("Keep the system files in the report"),
    llvm::cl::init(false)
);
static llvm::cl::alias tool_option_keep_system_short(
    "k",
    llvm::cl::cat(tool_category),
    llvm::cl::aliasopt(tool_option_output),
    llvm::cl::desc("Alias for --keep-system")
);


int main(int argc, const char* argv[]) {
    // create the parser for the tool
    auto parser = clang::tooling::CommonOptionsParser::create(argc, argv, tool_category);
    if (!parser) {
        llvm::errs() << llvm::toString(parser.takeError()) << '\n';
        return 1;
    }

    // fetch the parser options
    clang::tooling::CommonOptionsParser &options = parser.get();

    // create the tool from the options
    clang::tooling::ClangTool tool(options.getCompilations(), options.getSourcePathList());

    // create a consumer for the symbols
    auto consumer = std::make_shared<SymbolConsumer>(
        tool_option_keep_system.getValue()
    );

    // focus the indexer on declarations of symbols
    clang::index::IndexingOptions index_options;
    index_options.SystemSymbolFilter = clang::index::IndexingOptions::SystemSymbolFilterKind::DeclarationsOnly;
    index_options.IndexFunctionLocals = false;
    index_options.IndexImplicitInstantiation = false;
    SymbolActionFactory action_factory(consumer, index_options);

    // run the tool
    if (tool.run(&action_factory) == 1)
        throw std::runtime_error("could not index the source code.");

    // open the output file
    std::shared_ptr<llvm::raw_ostream> output_file = {
        &llvm::outs(),
        [](auto*) {}
    };  // by default, use the standard output

    std::string output_path = tool_option_output.getValue();
    if (output_path != "-") {
        std::error_code error_code;
        output_file = std::make_shared<llvm::raw_fd_ostream>(output_path, error_code);
        if (error_code) {
            llvm::errs() << "Cannot open output file: " << error_code.message() << '\n';
            return EXIT_FAILURE;
        }
    }

    // get the result document and output it
    const nlohmann::json& document = consumer->getDocument();
    *output_file << document.dump(
        4,
        ' ',
        false,
        nlohmann::json::error_handler_t::strict
    );

    return EXIT_SUCCESS;
}
