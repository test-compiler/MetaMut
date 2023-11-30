#ifndef YSMUT_EXPR_COPYEXPR_H
#define YSMUT_EXPR_COPYEXPR_H

#include "Mutator.h"
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

namespace ysmut {

class CopyExpr
    : public Mutator,
      public clang::RecursiveASTVisitor<CopyExpr> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitExpr(clang::Expr *E);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DR);
  bool VisitIntegerLiteral(clang::IntegerLiteral *IL);

private:
  std::vector<clang::Expr *> TheExprs;
  std::vector<clang::Expr *> TheOldVars;
};

} // namespace ysmut

#endif
