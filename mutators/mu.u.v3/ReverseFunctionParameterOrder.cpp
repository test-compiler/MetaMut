#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Expr.h>

using namespace clang;

class ReverseFunctionParameterOrder : public Mutator,
                      public clang::RecursiveASTVisitor<ReverseFunctionParameterOrder> {
public:
    using Mutator::Mutator;

    bool VisitFunctionDecl(FunctionDecl *FD) {
        if (FD->doesThisDeclarationHaveABody() && FD->getNumParams() > 1) {
            TheFuncs.push_back(FD);
        }
        return true;
    }

    bool mutate() override {
        TraverseDecl(getASTContext().getTranslationUnitDecl());

        if (TheFuncs.empty()) {
            return false;
        }

        FunctionDecl *FD = randElement(TheFuncs);
        auto Params = FD->parameters();
        std::string newParamList;

        // Generate the reversed parameter list
        for (auto it = Params.rbegin(); it != Params.rend(); ++it) {
            if (it != Params.rbegin())
                newParamList += ", ";
            newParamList += (*it)->getOriginalType().getAsString() + " " + (*it)->getNameAsString();
        }

        // Replace the original parameter list with the new one
        SourceLocation start = FD->getParamDecl(0)->getBeginLoc();
        SourceLocation end = FD->getParamDecl(FD->getNumParams() - 1)->getEndLoc();
        getRewriter().ReplaceText(SourceRange(start, end), newParamList);

        // Visit all function call expressions and reverse their arguments
        TraverseDecl(getASTContext().getTranslationUnitDecl());
        
        return true;
    }

    bool VisitCallExpr(CallExpr *CE) {
        FunctionDecl *Callee = CE->getDirectCallee();
        if (!Callee)
            return true;

        auto it = std::find(TheFuncs.begin(), TheFuncs.end(), Callee);
        if (it != TheFuncs.end()) {
            std::vector<Expr*> Args(CE->arguments().begin(), CE->arguments().end());
            std::string newArgList;

            // Generate the reversed argument list
            for (auto it = Args.rbegin(); it != Args.rend(); ++it) {
                if (it != Args.rbegin())
                    newArgList += ", ";
                newArgList += getSourceText(*it);
            }

            // Replace the original argument list with the new one
            SourceLocation start = CE->getArg(0)->getBeginLoc();
            SourceLocation end = CE->getArg(CE->getNumArgs() - 1)->getEndLoc();
            getRewriter().ReplaceText(SourceRange(start, end), newArgList);
        }

        return true;
    }

private:
    std::vector<FunctionDecl *> TheFuncs;
};

static RegisterMutator<ReverseFunctionParameterOrder> M("u3.ReverseFunctionParameterOrder", "This mutator selects a function and reverses the order of its parameters in both its declaration and all its calls.");
