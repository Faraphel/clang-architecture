#include <catch2/catch_test_macros.hpp>
#include <clang/Index/IndexingOptions.h>
#include <clang/Tooling/Tooling.h>
#include <nlohmann/json.hpp>

#include "SymbolActionFactory.hpp"
#include "SymbolConsumer.hpp"


static nlohmann::json run() {
    const auto consumer = std::make_shared<SymbolConsumer>();

    clang::index::IndexingOptions index_options;
    index_options.SystemSymbolFilter = clang::index::IndexingOptions::SystemSymbolFilterKind::DeclarationsOnly;
    index_options.IndexFunctionLocals = false;
    index_options.IndexImplicitInstantiation = false;
    SymbolActionFactory action_factory(consumer, index_options);

    const bool status = clang::tooling::runToolOnCode(
        action_factory.create(),
        R"(
                int main(const int argc, const char* argv[]) {
                    return 0;
                }
            )",
        "test.cpp"
    );

    if (!status)
        throw std::runtime_error("could not index the source code.");

    // get the output document
    return consumer->getDocument();
}


TEST_CASE("Index simple function") {
    auto document = run();

    SECTION("version") {
        REQUIRE(document.contains("version"));
        REQUIRE(document.at("version") == 1);
    }

    SECTION("files") {
        REQUIRE(document.contains("files"));

        const auto& files = document.at("files");

        REQUIRE(files.contains("test.cpp"));

        const auto& file = files.at("test.cpp");

        REQUIRE(file.contains("system"));
        REQUIRE(file.at("system") == false);
    }

    SECTION("references") {
        REQUIRE(document.contains("references"));

        const auto& references = document.at("references");

        REQUIRE(references.empty());
    }

    SECTION("symbols") {
        REQUIRE(document.contains("symbols"));

        const auto& symbols = document.at("symbols");

        REQUIRE(symbols.contains("c:@F@main#I#**1C#"));

        const auto& main = symbols.at("c:@F@main#I#**1C#");

        REQUIRE(main.contains("column"));
        REQUIRE(main.at("column") == 21);

        REQUIRE(main.contains("file"));
        REQUIRE(main.at("file") == "test.cpp");

        REQUIRE(main.contains("kind"));
        REQUIRE(main.at("kind") == "Function");

        REQUIRE(main.contains("line"));
        REQUIRE(main.at("line") == 2);

        REQUIRE(main.contains("name"));
        REQUIRE(main.at("name") == "main");

        REQUIRE(main.contains("system"));
        REQUIRE(main.at("system") == false);
    }
}
