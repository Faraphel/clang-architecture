#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/Index/IndexDataConsumer.h>
#include <clang/Index/IndexSymbol.h>
#include <clang/Sema/DeclSpec.h>
#include <nlohmann/json.hpp>


/**
 * Consume the symbols found while indexing a translation unit.
 */
class SymbolConsumer : public clang::index::IndexDataConsumer {
public:
    SymbolConsumer();

    void initialize(clang::ASTContext& context) override;

    bool handleDeclOccurrence(
        const clang::Decl *declaration,
        clang::index::SymbolRoleSet roles,
        llvm::ArrayRef<clang::index::SymbolRelation> relations,
        clang::SourceLocation raw_location,
        ASTNodeInfo node
    ) override;

    const nlohmann::json& getDocument() const;

private:
    const clang::ASTContext *context = nullptr;  /// the AST context
    nlohmann::json document;  /// the resulting JSON document
};
