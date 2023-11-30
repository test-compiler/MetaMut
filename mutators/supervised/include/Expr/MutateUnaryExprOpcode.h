#ifndef YSMUT_EXPR_MUTATEUNARYOPCODE_H
#define YSMUT_EXPR_MUTATEUNARYOPCODE_H

#include "Mutator.h"
#include <clang/AST/Expr.h>
#include <string>

namespace ysmut {

class MutateUnaryExprOpcode
    : public Mutator,
      public clang::RecursiveASTVisitor<MutateUnaryExprOpcode> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitUnaryOperator(clang::UnaryOperator *UO);

private:
  std::vector<clang::UnaryOperator *> TheOperators;

  std::vector<clang::UnaryOperatorKind> computeValidAlternatives(
      clang::UnaryOperator *op);
};

} // namespace ysmut

#endif
