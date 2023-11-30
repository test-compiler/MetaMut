#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Expr/IncreaseArrayDimension.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<IncreaseArrayDimension> M(
    "increase-array-dimension", "Add a dimension to an array variable.");

bool IncreaseArrayDimension::VisitVarDecl(VarDecl *VD) {
  if (isMutationSite(VD) && VD->getType()->isArrayType()) TheVars.push_back(VD);
  return true;
}

bool IncreaseArrayDimension::VisitArraySubscriptExpr(ArraySubscriptExpr *ASE) {
  if (isMutationSite(ASE)) {
    DeclRefExpr *refExpr =
        dyn_cast_or_null<DeclRefExpr>(ASE->getBase()->IgnoreParenImpCasts());
    if (refExpr) {
      VarDecl *var = dyn_cast_or_null<VarDecl>(refExpr->getDecl());
      if (var) VarToExprs[var].push_back(ASE);
    }
  }
  return true;
}

bool IncreaseArrayDimension::mutate() {
  TraverseAST(getASTContext());
  if (TheVars.empty() || VarToExprs.empty()) return false;

  VarDecl *var = randElement(TheVars);

  // Ensure the selected variable has corresponding ArraySubscriptExprs
  if (VarToExprs.find(var) == VarToExprs.end()) return false;

  // Add a dimension to the array variable
  SourceLocation endLoc =
      var->getLocation().getLocWithOffset(var->getNameAsString().size());
  getRewriter().InsertTextAfter(endLoc, "[1]");

  // Replace all corresponding ArraySubscriptExprs with an extra index
  for (auto *expr : VarToExprs[var]) {
    SourceLocation insLoc =
        expr->getBeginLoc().getLocWithOffset(var->getNameAsString().size());
    getRewriter().InsertTextBefore(insLoc, "[0]");
  }

  return true;
}
