#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class SwapVarInitExpr
    : public Mutator,
      public clang::RecursiveASTVisitor<SwapVarInitExpr> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVarDecls.size() < 2) return false;

    VarDecl *var1 = randElement(TheVarDecls);
    VarDecl *var2 = randElement(TheVarDecls);

    if (var1 == var2) return false;

    Expr *init1 = var1->getInit();
    Expr *init2 = var2->getInit();
    if (!init1 || !init2) return false;

    // Rewrite the variable initializations
    getRewriter().ReplaceText(var1->getInit()->getSourceRange(), getSourceText(init2));
    getRewriter().ReplaceText(var2->getInit()->getSourceRange(), getSourceText(init1));
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    if(VD->getInit())
      TheVarDecls.push_back(VD);
    return true;
  }

private:
  std::vector<clang::VarDecl *> TheVarDecls;
};

static RegisterMutator<SwapVarInitExpr> M("u3.SwapVarInitExpr", "Swap the initialization expressions of two local variables.");
