#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ModifyRandomFieldOfStructMutator : public Mutator,
    public clang::RecursiveASTVisitor<ModifyRandomFieldOfStructMutator> {
public:
    ModifyRandomFieldOfStructMutator(const char *name, const char *desc)
        : Mutator(name, desc) {}

    bool VisitFieldDecl(FieldDecl *fieldDecl) {
        if (mutationDone) {
            // Stop recursion if mutation is done
            return false;
        }

        fields.push_back(fieldDecl);
        return true;
    }

    bool mutate() override {
        TraverseDecl(context->getTranslationUnitDecl());

        if (fields.empty()) {
            return false;
        }

        // Randomly select a field
        FieldDecl *selectedField = fields.at(rand() % fields.size());

        // Change its type
        std::string newType = (selectedField->getType().getAsString() == "int") ? "double" : "int";
        SourceRange replacementRange(selectedField->getTypeSpecStartLoc(), selectedField->getLocation());
        rewriter->ReplaceText(replacementRange, newType);

        mutationDone = true;
        return true;
    }

    void setRewriter(Rewriter *rewriter) {
        this->rewriter = rewriter;
    }

    void setContext(ASTContext *context) {
        this->context = context;
    }

private:
    std::vector<FieldDecl*> fields;
    ASTContext *context = nullptr;
    Rewriter *rewriter = nullptr;
    bool mutationDone = false;
};

static RegisterMutator<ModifyRandomFieldOfStructMutator> X("ModifyRandomFieldOfStructMutator", "This mutator modifies a random field of a struct.");