#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/Stmt.h>
#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;

class ReplaceWhileWithDoWhile : public Mutator,
                                public clang::RecursiveASTVisitor<ReplaceWhileWithDoWhile> {
public:
    using Mutator::Mutator;

    bool VisitWhileStmt(clang::WhileStmt *S) {
        TheLoops.push_back(S);
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheLoops.empty()) return false;

        WhileStmt *WS = randElement(TheLoops);

        // Getting the while loop content
        Stmt *body = WS->getBody();
        std::string bodyText = getSourceText(body);

        // Getting the condition
        Expr *cond = WS->getCond();
        std::string condText = getSourceText(cond);

        // Prepare the do-while loop
        std::string doWhileLoop = "do {" + bodyText + "} while (" + condText + ");";

        // Replacing the while loop with do-while loop
        getRewriter().ReplaceText(WS->getSourceRange(), doWhileLoop);

        return true;
    }

private:
    std::vector<WhileStmt*> TheLoops;
};

static RegisterMutator<ReplaceWhileWithDoWhile> M("u3.ReplaceWhileWithDoWhile",
    "Replace while loop with equivalent do-while loop");
