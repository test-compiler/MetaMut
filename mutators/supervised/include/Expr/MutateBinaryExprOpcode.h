#ifndef YSMUT_EXPR_MUTATEBINARYEXPROPCODE_H
#define YSMUT_EXPR_MUTATEBINARYEXPROPCODE_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class MutateBinaryExprOpcode
    : public Mutator,
      public clang::RecursiveASTVisitor<MutateBinaryExprOpcode> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitBinaryOperator(clang::BinaryOperator *BO);

private:
  std::vector<clang::BinaryOperator *> TheOperators;

  std::vector<clang::BinaryOperatorKind> computeValidAlternatives(
      clang::BinaryOperator *op);
};

} // namespace ysmut

#endif
