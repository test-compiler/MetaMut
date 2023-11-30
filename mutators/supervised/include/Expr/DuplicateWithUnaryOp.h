#ifndef YSMUT_EXPR_DUPLICATEWITHUNARYOP_H
#define YSMUT_EXPR_DUPLICATEWITHUNARYOP_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include "Mutator.h"

namespace ysmut {

class DuplicateWithUnaryOp : public Mutator,
                      public clang::RecursiveASTVisitor<DuplicateWithUnaryOp> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitExpr(clang::Expr *E);

private:
  std::vector<clang::Expr *> TheExprs;
};

} // namespace ysmut

#endif
