#include "SymbolConsumer.hpp"

#include <clang/Index/USRGeneration.h>
#include <nlohmann/json.hpp>

#include "types.hpp"


constexpr auto reference_role = static_cast<clang::index::SymbolRoleSet>(clang::index::SymbolRole::Reference);
constexpr auto declaration_role = static_cast<clang::index::SymbolRoleSet>(clang::index::SymbolRole::Declaration);


void SymbolConsumer::initialize(clang::ASTContext& context) {
    this->context = &context;

    this->document = nlohmann::json::object({
        {"files", nlohmann::json::object()},
        {"symbols", nlohmann::json::object()},
        {"references", nlohmann::json::array()}
    });
}

bool SymbolConsumer::handleDeclOccurrence(
    const clang::Decl* declaration,
    const clang::index::SymbolRoleSet roles,
    llvm::ArrayRef<clang::index::SymbolRelation> relations,
    const clang::SourceLocation raw_location,
    ASTNodeInfo node
) {
    // get the source manager
    const clang::SourceManager &source_manager = this->context->getSourceManager();

    // get the location of the occurrence
    const clang::SourceLocation occurrence_location = source_manager.getExpansionLoc(raw_location);
    if (occurrence_location.isInvalid())
        return true;

    // get the exact information about the place where the symbol is used
    const clang::PresumedLoc occurrence_presumed = source_manager.getPresumedLoc(occurrence_location);
    if (occurrence_presumed.isInvalid())
        return true;

    // get the Unified Symbol Resolution (USR) for the symbol
    // this is a unique identifier across the different translation units for this symbol
    std::string symbol_usr;
    {
        usr_t symbol_usr_raw;
        if (clang::index::generateUSRForDecl(declaration, symbol_usr_raw))
            return true;

        symbol_usr = symbol_usr_raw.str().str();
    }

    std::string symbol_name = "?";
    if (const auto *named_declaration = llvm::dyn_cast<clang::NamedDecl>(declaration); named_declaration != nullptr)
        symbol_name = named_declaration->getNameAsString();

    // register the file
    {
        // check if the file is already registered
        const std::string file_name = occurrence_presumed.getFilename();
        if (!this->document["files"].contains(file_name)) {
            // if not already registered, add a record
            this->document["files"][file_name] = nlohmann::json::object({
                {"system", source_manager.isInSystemHeader(occurrence_location)}
            });
        }
    }

    // check for the kind of usage of this symbol
    if (roles & declaration_role) {

        // skip if the symbol is already registered
        if (this->document["symbols"].contains(symbol_usr))
            return true;

        // register the symbol
        this->document["symbols"][symbol_usr] = nlohmann::json::object({
            {"name", symbol_name},
            {"kind", declaration->getDeclKindName()},
            {"system", source_manager.isInSystemHeader(occurrence_location)},
            {"file", occurrence_presumed.getFilename()},
            {"line", occurrence_presumed.getLine()},
            {"column", occurrence_presumed.getColumn()}
        });

    } else if (roles & reference_role) {

        // register the symbol reference
        this->document["references"].push_back(nlohmann::json::object({
            {"usr", symbol_usr},
            {"file", occurrence_presumed.getFilename()},
            {"line", occurrence_presumed.getLine()},
            {"column", occurrence_presumed.getColumn()}
        }));

    }

    return true;
}


const nlohmann::json& SymbolConsumer::getDocument() const {
    return this->document;
}