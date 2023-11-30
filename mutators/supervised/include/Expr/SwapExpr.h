#ifndef YSMUT_EXPR_SWAPEXPR_H
#define YSMUT_EXPR_SWAPEXPR_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class SwapExpr
    : public Mutator,
      public clang::RecursiveASTVisitor<SwapExpr> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitExpr(clang::Expr *E);

private:
  std::vector<clang::Expr *> TheExpressions;

  bool areCrossAssignable(clang::Expr *lhs, clang::Expr *rhs);
};

} // namespace ysmut

#endif
