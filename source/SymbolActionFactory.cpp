#include "SymbolActionFactory.hpp"

#include <clang/Index/IndexingAction.h>


SymbolActionFactory::SymbolActionFactory(
        std::shared_ptr<clang::index::IndexDataConsumer> consumer,
        const clang::index::IndexingOptions &options
) {
    this->consumer = std::move(consumer);
    this->options = options;
}


std::unique_ptr<clang::FrontendAction> SymbolActionFactory::create() {
    return clang::index::createIndexingAction(this->consumer, this->options);
}