#include <cstdlib>
#include <ctime>
#include <sstream>

#include "Mutator.h"
#include "MutatorManager.h"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;
using namespace ysmut;

class RandomizeZeroArgumentValues : public Mutator,
                                    public clang::RecursiveASTVisitor<RandomizeZeroArgumentValues> {
private:
    ASTContext *Context = nullptr;
    Rewriter *R = nullptr;

public:
    RandomizeZeroArgumentValues(const char* name, const char* desc) : Mutator(name, desc) {
        std::srand(std::time(nullptr));
    }

    void setContextAndRewriter(ASTContext *Context, Rewriter *R) {
        this->Context = Context;
        this->R = R;
    }

    bool VisitCallExpr(CallExpr *callExpr) {
        if (!Context || !R) return true;

        for (int i = 0; i < callExpr->getNumArgs(); i++) {
            Expr *arg = callExpr->getArg(i);
            if (isa<IntegerLiteral>(arg)) {
                IntegerLiteral* intLit = cast<IntegerLiteral>(arg);
                if (intLit->getValue() == 0) {
                    std::stringstream ss;
                    ss << (std::rand() % 100 + 1); // random integer between 1 and 100
                    R->ReplaceText(arg->getSourceRange(), ss.str());
                }
            }
        }
        return true;
    }

    bool mutate() override {
        if (!Context) return false;
        return TraverseDecl(Context->getTranslationUnitDecl());
    }
};

static RegisterMutator<RandomizeZeroArgumentValues> Y("RandomizeZeroArgumentValues", "Replace zero arguments in function calls with a random integer");