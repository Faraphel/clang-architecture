#include "SymbolConsumer.hpp"

#include <clang/Index/USRGeneration.h>
#include <nlohmann/json.hpp>

#include "types.hpp"


constexpr auto reference_role = static_cast<clang::index::SymbolRoleSet>(clang::index::SymbolRole::Reference);
constexpr auto declaration_role = static_cast<clang::index::SymbolRoleSet>(clang::index::SymbolRole::Declaration);
constexpr auto definition_role = static_cast<clang::index::SymbolRoleSet>(clang::index::SymbolRole::Definition);


SymbolConsumer::SymbolConsumer() {
    this->document = nlohmann::json::object({
        {"version", 1},
        {"files", nlohmann::json::object()},
        {"symbols", nlohmann::json::object()},
        {"references", nlohmann::json::array()}
    });
}

void SymbolConsumer::initialize(clang::ASTContext& context) {
    this->context = &context;
}


bool SymbolConsumer::handleAnyOccurrence(
    const std::string& usr,
    const std::string& name,
    const std::string& kind,
    const clang::index::SymbolRoleSet& roles,
    const clang::SourceLocation& location,
    const clang::PresumedLoc& presumed
) {
    // get the source manager
    const clang::SourceManager &source_manager = this->context->getSourceManager();

    // register the file
    {
        // check if the file is already registered
        const std::string file_name = presumed.getFilename();
        if (!this->document["files"].contains(file_name)) {
            // if not already registered, add a record
            this->document["files"][file_name] = nlohmann::json::object({
                {"system", source_manager.isInSystemHeader(location)}
            });
        }
    }

    // check for the kind of usage of this symbol
    if (roles & (declaration_role | definition_role)) {

        // skip if the symbol is already registered
        if (this->document["symbols"].contains(usr))
            return true;

        // register the symbol
        this->document["symbols"][usr] = nlohmann::json::object({
            {"name", name},
            {"kind", kind},
            {"system", source_manager.isInSystemHeader(location)},
            {"file", presumed.getFilename()},
            {"line", presumed.getLine()},
            {"column", presumed.getColumn()}
        });

    } else if (roles & reference_role) {

        // register the symbol reference
        this->document["references"].push_back(nlohmann::json::object({
            {"usr", usr},
            {"file", presumed.getFilename()},
            {"line", presumed.getLine()},
            {"column", presumed.getColumn()}
        }));

    }

    return true;
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
    const clang::SourceLocation location = source_manager.getExpansionLoc(raw_location);
    if (location.isInvalid())
        return true;

    // get the exact information about the place where the symbol is used
    const clang::PresumedLoc presumed = source_manager.getPresumedLoc(location);
    if (presumed.isInvalid())
        return true;

    // get the Unified Symbol Resolution (USR) for the symbol
    // this is a unique identifier across the different translation units for this symbol
    std::string usr;
    {
        usr_t usr_raw;
        if (clang::index::generateUSRForDecl(declaration, usr_raw))
            return true;

        usr = usr_raw.str().str();
    }

    // get the name of the symbol
    std::string name = "?";
    if (const auto *named_declaration = llvm::dyn_cast<clang::NamedDecl>(declaration); named_declaration != nullptr)
        name = named_declaration->getNameAsString();

    // delegate to the generic handler
    return this->handleAnyOccurrence(
        usr,
        name,
        declaration->getDeclKindName(),
        roles,
        location,
        presumed
    );
}


bool SymbolConsumer::handleMacroOccurrence(
    const clang::IdentifierInfo* identifier,
    const clang::MacroInfo* macro,
    clang::index::SymbolRoleSet roles,
    clang::SourceLocation raw_location
) {
    // get the source manager
    const clang::SourceManager &source_manager = this->context->getSourceManager();

    // get the location of the occurrence
    const clang::SourceLocation location = source_manager.getExpansionLoc(raw_location);
    if (location.isInvalid())
        return true;

    // get the exact information about the place where the macro is used
    const clang::PresumedLoc presumed = source_manager.getPresumedLoc(location);
    if (presumed.isInvalid())
        return true;

    // get the Unified Symbol Resolution (USR) for the macro
    // this is a unique identifier across the different translation units for this macro
    std::string usr;
    {
        usr_t usr_raw;
        if (clang::index::generateUSRForMacro(
            identifier->getName(),
            macro->getDefinitionLoc(),
            source_manager,
            usr_raw
        ))
            return true;

        usr = usr_raw.str().str();
    }

    // get the name of the macro
    const std::string name = identifier->getName().str();

    // delegate to the generic handler
    return this->handleAnyOccurrence(
        usr,
        name,
        "Macro",
        roles,
        location,
        presumed
    );
}


const nlohmann::json& SymbolConsumer::getDocument() const {
    return this->document;
}