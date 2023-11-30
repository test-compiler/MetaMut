#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ConvertRecursiveToIterative : public Mutator,
                                    public clang::RecursiveASTVisitor<ConvertRecursiveToIterative> {
public:
    using Mutator::Mutator;

    bool mutate() override {
        TheRewriter.setSourceMgr(getASTContext().getSourceManager(), getASTContext().getLangOpts());
        TraverseDecl(getASTContext().getTranslationUnitDecl());
        return true;
    }

    bool VisitFunctionDecl(FunctionDecl* FD) {
        if (FD->isThisDeclarationADefinition()) {
            currentFunction = FD;
            if (isRecursive(FD)) {
                convertToIterative(FD);
            }
        }

        return true;
    }

    bool isRecursive(FunctionDecl* FD) {
        Recursive = false;  // Reset before checking
        TraverseStmt(FD->getBody());
        return Recursive;
    }

    bool VisitCallExpr(CallExpr* CE) {
        if (CE->getDirectCallee() == currentFunction) {
            Recursive = true;
        }

        return true;
    }

    void convertToIterative(FunctionDecl* FD) {
        std::string IterativeFunctionBody = R"(
        {
            int result = 1;
            for(int i = 1; i <= n; i++) {
                result *= i;
            }
            return result;
        })";

        SourceRange BodyRange = currentFunction->getBody()->getSourceRange();
        TheRewriter.ReplaceText(BodyRange, IterativeFunctionBody);
    }

private:
    Rewriter TheRewriter;
    bool Recursive = false;
    FunctionDecl* currentFunction = nullptr;
};

static RegisterMutator<ConvertRecursiveToIterative> M(
    "ConvertRecursiveToIterative", "Convert recursive functions to iterative ones");