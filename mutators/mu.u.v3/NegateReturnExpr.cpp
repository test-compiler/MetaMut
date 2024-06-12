#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class NegateReturnExpr : public Mutator,
                       public clang::RecursiveASTVisitor<NegateReturnExpr> {
public:
    using Mutator::Mutator;

    bool VisitReturnStmt(ReturnStmt* stmt) {
        if (stmt->getRetValue()->getType()->isIntegerType() ||
            stmt->getRetValue()->getType()->isBooleanType()) {
            TheReturnStmts.push_back(stmt);
        }
        return true;
    }

    bool mutate() override {
        // Traverse the AST
        TraverseAST(getASTContext());

        // If no valid Return statements found, return false
        if (TheReturnStmts.empty())
            return false;

        // Select a random Return statement from the collected statements
        ReturnStmt* selectedStmt = randElement(TheReturnStmts);

        // Get the return value and its source text
        Expr* returnValue = selectedStmt->getRetValue();
        std::string returnValueText = getSourceText(returnValue);

        // Perform the negation and return true
        getRewriter().ReplaceText(returnValue->getSourceRange(), "!(" + returnValueText + ")");
        return true;
    }

private:
    std::vector<ReturnStmt*> TheReturnStmts; // Store the Return statements
};

static RegisterMutator<NegateReturnExpr> M("u3.NegateReturnExpr", "Negate the return expression if it is a boolean or an integer.");
