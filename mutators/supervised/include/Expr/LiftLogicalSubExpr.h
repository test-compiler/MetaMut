#ifndef YSMUT_EXPR_LIFTLOGICALSUBEXPR_H
#define YSMUT_EXPR_LIFTLOGICALSUBEXPR_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class LiftLogicalSubExpr : public Mutator,
                           public clang::RecursiveASTVisitor<LiftLogicalSubExpr> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitBinaryOperator(clang::BinaryOperator *BO);

private:
  std::vector<clang::BinaryOperator *> BinaryOps;
};

} // namespace ysmut

#endif
