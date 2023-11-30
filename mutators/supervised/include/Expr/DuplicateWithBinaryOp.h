#ifndef YSMUT_EXPR_DUPLICATEWITHBINARYOP_H
#define YSMUT_EXPR_DUPLICATEWITHBINARYOP_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class DuplicateWithBinaryOp : public Mutator,
                              public clang::RecursiveASTVisitor<DuplicateWithBinaryOp> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitExpr(clang::Expr *E);

private:
  std::vector<clang::Expr *> TheExprs;
};

} // namespace ysmut

#endif
