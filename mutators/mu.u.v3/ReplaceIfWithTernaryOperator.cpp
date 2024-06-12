#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceIfWithTernaryOperator : public Mutator,
                                     public RecursiveASTVisitor<ReplaceIfWithTernaryOperator> {
public:
    using Mutator::Mutator;

    ReturnStmt* getReturnStmt(Stmt* stmt) {
        if (isa<ReturnStmt>(stmt))
            return cast<ReturnStmt>(stmt);
        else if (isa<CompoundStmt>(stmt)) {
            for (auto* substmt : cast<CompoundStmt>(stmt)->body()) {
                ReturnStmt* returnStmt = getReturnStmt(substmt);
                if (returnStmt) return returnStmt;
            }
        }
        return nullptr;
    }

    bool VisitIfStmt(IfStmt *IF) {
        // Collect 'if-else' statements that have 'return' in both branches
        Stmt *thenStmt = IF->getThen();
        Stmt *elseStmt = IF->getElse();
        ReturnStmt *thenReturn = nullptr;
        ReturnStmt *elseReturn = nullptr;

        if (thenStmt)
            thenReturn = getReturnStmt(thenStmt);
        if (elseStmt)
            elseReturn = getReturnStmt(elseStmt);

        if (thenReturn && elseReturn)
            IfStmts.push_back(IF);
        return true;
    }

    bool mutate() override {
        // Step 1: Traverse the AST
        TraverseAST(getASTContext());

        // Step 2: Check if there are eligible 'if-else' statements
        if (IfStmts.empty()) return false;

        // Step 3: Randomly select an 'if-else' statement
        IfStmt *IF = randElement(IfStmts);

        // Step 4: Generate a replacement with a ternary conditional operator
        std::string replacement = "return " + getSourceText(IF->getCond()) + " ? " +
                                  getSourceText(getReturnStmt(IF->getThen())->getRetValue()) + " : " +
                                  getSourceText(getReturnStmt(IF->getElse())->getRetValue()) + ";";

        // Step 5: Replace the 'if-else' statement with the replacement
        getRewriter().ReplaceText(IF->getSourceRange(), replacement);

        return true;
    }

private:
    std::vector<IfStmt*> IfStmts;
};

static RegisterMutator<ReplaceIfWithTernaryOperator> M("u3.ReplaceIfWithTernaryOperator", "Replace an 'if-else' statement with a ternary conditional operator");
