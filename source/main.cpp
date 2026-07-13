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
    llvm::cl::Optional,
    llvm::cl::desc("output table path"),
    llvm::cl::value_desc("path"),
    llvm::cl::init(std::filesystem::path("/dev/stdout")),
    llvm::cl::cat(tool_category)
);
static llvm::cl::alias tool_option_output_short(
    "o",
    llvm::cl::desc("Alias for --output"),
    llvm::cl::aliasopt(tool_option_output),
    llvm::cl::cat(tool_category)
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
    auto consumer = std::make_shared<SymbolConsumer>();

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
    std::filesystem::path output_path = tool_option_output.getValue();
    std::ofstream output_file(output_path);

    // get the result document and output it
    const nlohmann::json& document = consumer->getDocument();
    output_file << document.dump(
        4,
        ' ',
        false,
        nlohmann::json::error_handler_t::strict
    );

    return EXIT_SUCCESS;
}
