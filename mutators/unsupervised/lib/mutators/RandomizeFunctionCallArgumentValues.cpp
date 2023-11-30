#include <cstdlib>
#include <cstring>
#include <ctime>

#include "Mutator.h"
#include "MutatorManager.h"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;
using namespace ysmut;

class RandomizeFunctionCallArgumentValues : public Mutator,
                                            public clang::RecursiveASTVisitor<RandomizeFunctionCallArgumentValues> {
private:
    ASTContext *Context = nullptr;
    Rewriter *R = nullptr;

public:
    RandomizeFunctionCallArgumentValues(const char* name, const char* desc) : Mutator(name, desc) {
        std::srand(std::time(nullptr));
    }

    void setContextAndRewriter(ASTContext *Context, Rewriter *R) {
        this->Context = Context;
        this->R = R;
    }

    bool VisitCallExpr(CallExpr *callExpr) {
        if (!Context || !R) return true;

        if (callExpr->getNumArgs() > 0) {
            for (int i = 0; i < callExpr->getNumArgs(); i++) {
                Expr *arg = callExpr->getArg(i);
                if (isa<CXXNullPtrLiteralExpr>(arg)) {
                    char randomStr[8];
                    for (int i = 0; i < 7; i++) {
                        randomStr[i] = 'a' + std::rand() % 26;
                    }
                    randomStr[7] = '\0';
                    R->ReplaceText(arg->getSourceRange(), "\"" + std::string(randomStr) + "\"");
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

static RegisterMutator<RandomizeFunctionCallArgumentValues> Y("RandomizeFunctionCallArgumentValues", "Randomizes the values of arguments in function calls");