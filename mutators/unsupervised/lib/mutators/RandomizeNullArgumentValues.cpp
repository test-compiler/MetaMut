#include "Mutator.h"
#include "MutatorManager.h"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;
using namespace ysmut;

class RandomizeNullArgumentValues : public Mutator,
                                    public clang::RecursiveASTVisitor<RandomizeNullArgumentValues> {
private:
    ASTContext *Context = nullptr;
    Rewriter *R = nullptr;

public:
    RandomizeNullArgumentValues(const char* name, const char* desc) : Mutator(name, desc) {}

    void setContextAndRewriter(ASTContext *Context, Rewriter *R) {
        this->Context = Context;
        this->R = R;
    }

    bool VisitCallExpr(CallExpr *callExpr) {
        if (!Context || !R) return true;

        for (int i = 0; i < callExpr->getNumArgs(); i++) {
            Expr *arg = callExpr->getArg(i);
            if (isa<clang::CXXNullPtrLiteralExpr>(arg)) {
                R->ReplaceText(arg->getSourceRange(), "\"RandomString\"");
            }
        }
        return true;
    }

    bool mutate() override {
        if (!Context) return false;
        return TraverseDecl(Context->getTranslationUnitDecl());
    }
};

static RegisterMutator<RandomizeNullArgumentValues> Y("RandomizeNullArgumentValues", "Replace NULL arguments in function calls with a random string");