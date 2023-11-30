#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Expr/SwapExpr.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<SwapExpr> M(
    "swap-expr", "Swap two expressions if they are type compliant.");

bool SwapExpr::VisitExpr(Expr *E) {
  if (isMutationSite(E)) TheExpressions.push_back(E);
  return true;
}

bool SwapExpr::mutate() {
  TraverseAST(getASTContext());
  if (TheExpressions.size() < 2) return false;

  Expr *expr1 = randElement(TheExpressions);
  Expr *expr2 = randElement(TheExpressions);

  if (expr1 == expr2 || !areCrossAssignable(expr1, expr2)) return false;

  // perform rewrite
  SourceRange expr1Range = expr1->getSourceRange();
  SourceRange expr2Range = expr2->getSourceRange();
  const auto &LangOpts = getASTContext().getLangOpts();

  if (rangesIntersect(
          getASTContext().getSourceManager(), expr1Range, expr2Range))
    return false;

  std::string expr1Text = getSourceText(expr1).str();
  std::string expr2Text = getSourceText(expr2).str();

  if (expr1Text == expr2Text) return false;

  getRewriter().ReplaceText(expr1Range, expr2Text);
  getRewriter().ReplaceText(expr2Range, expr1Text);

  return true;
}

bool SwapExpr::areCrossAssignable(Expr *lhs, Expr *rhs) {
  auto &sema = getCompilerInstance().getSema();
  return sema.CheckAssignmentConstraints(lhs->getBeginLoc(), lhs->getType(),
             rhs->getType()) == Sema::Compatible &&
         sema.CheckAssignmentConstraints(rhs->getBeginLoc(), rhs->getType(),
             lhs->getType()) == Sema::Compatible;
}
