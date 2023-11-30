#ifndef YSMUT_EXPR_OPCODE_REMOVEUNARYOPCODE_H
#define YSMUT_EXPR_OPCODE_REMOVEUNARYOPCODE_H

#include "Mutator.h"
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

namespace ysmut {

class RemoveUnaryExprOpcode
    : public Mutator,
      public clang::RecursiveASTVisitor<RemoveUnaryExprOpcode> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitUnaryOperator(clang::UnaryOperator *UO);

private:
  std::vector<const clang::UnaryOperator *> TheOperators;
};

} // namespace ysmut

#endif
