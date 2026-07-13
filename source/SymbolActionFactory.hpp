#pragma once

#include <memory>

#include <clang/Index/IndexDataConsumer.h>
#include <clang/Index/IndexingOptions.h>
#include <clang/Tooling/Tooling.h>


/**
 * Factory for a ClangTool FrontendAction that runs the consumer on the translation unit.
 */
class SymbolActionFactory : public clang::tooling::FrontendActionFactory {
public:
    SymbolActionFactory(
        std::shared_ptr<clang::index::IndexDataConsumer> consumer,
        const clang::index::IndexingOptions &options
    );

    std::unique_ptr<clang::FrontendAction> create() override;

private:
    std::shared_ptr<clang::index::IndexDataConsumer> consumer;  /// the consumer to use
    clang::index::IndexingOptions options;  /// the indexing options
};
