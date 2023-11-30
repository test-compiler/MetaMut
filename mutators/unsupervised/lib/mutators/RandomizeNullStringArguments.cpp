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

class RandomizeNullStringArguments : public Mutator,
                                     public clang::RecursiveASTVisitor<RandomizeNullStringArguments> {
private:
    ASTContext *Context = nullptr;
    Rewriter *R = nullptr;

public:
    RandomizeNullStringArguments(const char* name, const char* desc) : Mutator(name, desc) {
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
            if (isa<CXXNullPtrLiteralExpr>(arg)) {
                std::string randomStr = "\"RandomString\""; // Replace this with a function to generate a random string if desired.
                R->ReplaceText(arg->getSourceRange(), randomStr);
            }
        }
        return true;
    }

    bool mutate() override {
        if (!Context) return false;
        return TraverseDecl(Context->getTranslationUnitDecl());
    }
};

static RegisterMutator<RandomizeNullStringArguments> Y("RandomizeNullStringArguments", "Replace NULL string arguments in function calls with a random string");