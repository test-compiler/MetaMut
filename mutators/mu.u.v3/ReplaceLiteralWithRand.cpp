#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include <cstdlib>

using namespace clang;

class ReplaceLiteralWithRand : public Mutator,
                               public clang::RecursiveASTVisitor<ReplaceLiteralWithRand> {
public:
    using Mutator::Mutator;

    bool VisitIntegerLiteral(IntegerLiteral *IL) {
        TheLiterals.push_back(IL);
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheLiterals.empty()) return false;

        IntegerLiteral *expr = randElement(TheLiterals);

        // Replace the integer literal with a random number
        std::string randNum = std::to_string(rand());
        getRewriter().ReplaceText(expr->getSourceRange(), randNum);

        return true;
    }

private:
    std::vector<IntegerLiteral *> TheLiterals;
};

static RegisterMutator<ReplaceLiteralWithRand> M("u3.ReplaceLiteralWithRand", "Replaces a numeric literal with a randomly generated number.");
