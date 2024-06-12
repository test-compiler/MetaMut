#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class ChangeWhileToForLoop : public Mutator,
                             public clang::RecursiveASTVisitor<ChangeWhileToForLoop> {
public:
    using Mutator::Mutator;

    bool VisitWhileStmt(clang::WhileStmt *WS) {
        TheWhileStmts.push_back(WS);
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheWhileStmts.empty()) return false;

        clang::WhileStmt *SelectedStmt = randElement(TheWhileStmts);

        Stmt *body = SelectedStmt->getBody();
        CompoundStmt *CS = dyn_cast<CompoundStmt>(body);

        if (!CS || CS->body_empty()) return false;

        Stmt *initExpr = CS->body_back();

        // Create for loop statement based on the original while loop condition and the last statement in its body
        std::string ForLoop = "for (; " + getSourceText(SelectedStmt->getCond()) + "; " + getSourceText(initExpr) + ") ";

        // replace the entire while loop statement with the new for loop
        getRewriter().ReplaceText(SelectedStmt->getSourceRange(), ForLoop);

        return true;
    }

private:
    std::vector<clang::WhileStmt *> TheWhileStmts;
};

static RegisterMutator<ChangeWhileToForLoop> M("u3.ChangeWhileToForLoop", "Change 'while' loop into equivalent 'for' loop without changing loop semantics.");
