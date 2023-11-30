#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ModifyRandomFieldOfStruct : public Mutator,
                                  public RecursiveASTVisitor<ModifyRandomFieldOfStruct> {
public:
    ModifyRandomFieldOfStruct(const char *name, const char *desc)
        : Mutator(name, desc) {}

    // Visitor for FieldDecl
    bool VisitFieldDecl(FieldDecl *FD) {
        TheFields.push_back(FD);
        TheTypes.push_back(FD->getType());
        return true;
    }

    // Run before traversing the AST
    bool PreTraverse(ASTContext &Ctx, Rewriter &Rw) {
        TheContext = &Ctx;
        TheRewriter = &Rw;
        return true;
    }

    // Traverse the AST and collect all FieldDecl nodes
    bool TraverseDecl(Decl *D) {
        if (D) {
            RecursiveASTVisitor<ModifyRandomFieldOfStruct>::TraverseDecl(D);
        }
        return true;
    }

    // Mutation method
    bool mutate() override {
        if (TheFields.empty() || TheTypes.empty() || !TheContext || !TheRewriter) return false;

        FieldDecl *field = randElement(TheFields);
        QualType oldType = field->getType();

        std::vector<QualType> compatibleTypes;
        for (QualType type : TheTypes) {
            if (TheContext->getTypeSize(type) == TheContext->getTypeSize(oldType)) {
                compatibleTypes.push_back(type);
            }
        }

        if (compatibleTypes.empty()) return false;

        QualType newType = randElement(compatibleTypes);
        if (newType == oldType) return false;

        SourceLocation fieldStart = field->getBeginLoc();

        std::string fieldTypeText = Lexer::getSourceText(CharSourceRange::getTokenRange(fieldStart, field->getLocation()),
                                                         TheContext->getSourceManager(),
                                                         TheContext->getLangOpts()).str();
        SourceLocation fieldTypeEnd = fieldStart.getLocWithOffset(fieldTypeText.size());

        TheRewriter->ReplaceText(SourceRange(fieldStart, fieldTypeEnd), formatAsDecl(newType, ""));

        return true;
    }

private:
    std::vector<FieldDecl *> TheFields;
    std::vector<QualType> TheTypes;
    ASTContext *TheContext = nullptr;
    Rewriter *TheRewriter = nullptr;

    // Returns a random element from the given vector
    template <typename T>
    T randElement(const std::vector<T> &v) {
        return v[rand() % v.size()];
    }

    // Returns a string representation of a declaration with the given type and identifier
    std::string formatAsDecl(QualType type, std::string id) {
        return type.getAsString() + " " + id;
    }
};

static RegisterMutator<ModifyRandomFieldOfStruct> X(
    "ModifyRandomFieldOfStruct", 
    "Randomly selects a FieldDecl within a RecordDecl (Struct or Union), and changes its type to another compatible type from the existing types in the program."
);