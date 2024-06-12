#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class WrapWithTryCatch : public Mutator,
                         public clang::RecursiveASTVisitor<WrapWithTryCatch> {
public:
    using Mutator::Mutator;

    bool VisitCompoundStmt(CompoundStmt *S) {
        statements.push_back(S);
        return true;
    }

    bool mutate() override {
        TraverseDecl(getASTContext().getTranslationUnitDecl());

        if (statements.empty()) {
            return false;
        }

        Stmt *stmt = randElement(statements);
        std::string stmtText = getSourceText(stmt);
        std::string fakeTryCatchText = "{ int error = 0; if (error) { fprintf(stderr, \"An error occurred.\\n\"); } else " + stmtText + " }";
        getRewriter().ReplaceText(stmt->getSourceRange(), fakeTryCatchText);

        return true;
    }

private:
    std::vector<Stmt *> statements;
};

static RegisterMutator<WrapWithTryCatch> M("u3.WrapWithTryCatch", "Wraps a statement or a block of statements within a simulated try-catch block by introducing if-else statements.");
