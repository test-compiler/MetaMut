#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class SwapIfElseStmt : public Mutator,
                       public clang::RecursiveASTVisitor<SwapIfElseStmt> {
public:
    using Mutator::Mutator;

    bool VisitIfStmt(IfStmt* stmt) {
        if (stmt->getElse())
            TheIfElseStmts.push_back(stmt); 
        return true;
    }

    bool mutate() override {
        // Traverse the AST
        TraverseAST(getASTContext());

        // If no valid If-Else statements found, return false
        if (TheIfElseStmts.empty())
            return false;

        // Select a random If-Else statement from the collected statements
        IfStmt* selectedStmt = randElement(TheIfElseStmts);

        // Get the contents of the 'if' and 'else' branches
        std::string ifStmtText = getSourceText(selectedStmt->getThen());
        std::string elseStmtText = getSourceText(selectedStmt->getElse());

        // Swap the 'if' and 'else' branches
        getRewriter().ReplaceText(selectedStmt->getThen()->getSourceRange(), elseStmtText);
        getRewriter().ReplaceText(selectedStmt->getElse()->getSourceRange(), ifStmtText);

        return true;
    }

private:
    std::vector<IfStmt*> TheIfElseStmts; // Store the 'if-else' statements
};

static RegisterMutator<SwapIfElseStmt> M("u3.SwapIfElseStmt", "Swaps the block of statements in the 'if' branch with that in the 'else' branch.");
