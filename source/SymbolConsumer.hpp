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

    /**
     * Handle any kind of occurrence
     * @param usr the Unified Symbol Resolution (USR) of the occurrence
     * @param name the name of the occurrence
     * @param kind the kind of occurrence
     * @param roles the roles of the occurrence
     * @param location the location of the occurrence
     * @param presumed the source information of the occurrence
     * @return true if the consumer should keep iterate, false otherwise
     */
    bool handleAnyOccurrence(
        const std::string& usr,
        const std::string& name,
        const std::string& kind,
        const clang::index::SymbolRoleSet& roles,
        const clang::SourceLocation& location,
        const clang::PresumedLoc& presumed
    );

    bool handleDeclOccurrence(
        const clang::Decl *declaration,
        clang::index::SymbolRoleSet roles,
        llvm::ArrayRef<clang::index::SymbolRelation> relations,
        clang::SourceLocation raw_location,
        ASTNodeInfo node
    ) override;

    bool handleMacroOccurrence(
        const clang::IdentifierInfo* identifier,
        const clang::MacroInfo* macro,
        clang::index::SymbolRoleSet roles,
        clang::SourceLocation raw_location
    ) override;

    [[nodiscard]] const nlohmann::json& getDocument() const;

private:
    const clang::ASTContext *context = nullptr;  /// the AST context
    nlohmann::json document;  /// the resulting JSON document
};
