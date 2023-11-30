#include <ctime>
#include <cstdlib>

#include "Mutator.h"
#include "MutatorManager.h"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;
using namespace ysmut;

class RandomizeParamDefaultValues : public Mutator, public clang::RecursiveASTVisitor<RandomizeParamDefaultValues> {
private:
    ASTContext *Context = nullptr;
    Rewriter *R = nullptr;

public:
    RandomizeParamDefaultValues(const char* name, const char* desc) : Mutator(name, desc) {
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
            if (IntegerLiteral* intLiteral = dyn_cast<IntegerLiteral>(arg)) {
                if (intLiteral->getValue().getLimitedValue() == 0) {
                    std::string randomNum = std::to_string(std::rand() % 100 + 1); // random number between 1 and 100
                    R->ReplaceText(arg->getSourceRange(), randomNum);
                }
            }
        }
        return true;
    }

    bool mutate() override {
        if (!Context || !R) return false;
        return TraverseDecl(Context->getTranslationUnitDecl());
    }
};

static RegisterMutator<RandomizeParamDefaultValues> Y("RandomizeParamDefaultValues", "Replace zero integer arguments in function calls with a random number");