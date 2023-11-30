#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Expr/IncreasePointerLevel.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<IncreasePointerLevel> M(
    "increase-pointer-level", "Increase a variable's pointer level.");

bool IncreasePointerLevel::VisitVarDecl(VarDecl *VD) {
  TheVars.push_back(VD);
  return true;
}

bool IncreasePointerLevel::VisitDeclRefExpr(DeclRefExpr *DRE) {
  if (VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
    VarToRefs[VD].push_back(DRE);
  }
  return true;
}

bool IncreasePointerLevel::mutate() {
  TraverseAST(getASTContext());
  if (TheVars.empty()) return false;

  VarDecl *oldVar = randElement(TheVars);

  std::string oldVarName = oldVar->getNameAsString();
  std::string newVarName = "*" + oldVarName;

  // Replace old var name with new var name
  getRewriter().ReplaceText(oldVar->getLocation(), oldVarName.size(), newVarName);

  // Check if the VarDecl has an initializer
  if (oldVar->hasInit()) {
    Expr *initExpr = oldVar->getInit();

    // Check if the init expr is an lvalue
    if (initExpr->isLValue()) {
      getRewriter().InsertText(initExpr->getBeginLoc(), "&");
    } else {
      getRewriter().ReplaceText(initExpr->getSourceRange(), "(void *)0");
    }
  }

  // Replace all DeclRefExprs that refer to the old var
  for (auto *DRE : VarToRefs[oldVar]) {
    getRewriter().InsertTextBefore(DRE->getBeginLoc(), "*");
  }

  return true;
}
