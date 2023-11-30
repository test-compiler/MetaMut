#ifndef YSMUT_EXPR_SWAPCHILDEXPR_H
#define YSMUT_EXPR_SWAPCHILDEXPR_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class SwapChildExpr : public Mutator,
                      public clang::RecursiveASTVisitor<SwapChildExpr> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitBinaryOperator(clang::BinaryOperator *BO);

private:
  std::vector<clang::BinaryOperator *> TheOps;
};

} // namespace ysmut

#endif
