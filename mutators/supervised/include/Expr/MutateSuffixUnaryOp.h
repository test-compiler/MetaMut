#ifndef YSMUT_EXPR_MUTATESUFFIXUNARYOP_H
#define YSMUT_EXPR_MUTATESUFFIXUNARYOP_H

#include "Mutator.h"
#include <clang/AST/RecursiveASTVisitor.h>

namespace ysmut {

class MutateSuffixUnaryOp
    : public Mutator,
      public clang::RecursiveASTVisitor<MutateSuffixUnaryOp> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitUnaryOperator(clang::UnaryOperator *UO);

private:
  std::vector<clang::UnaryOperator *> TheUnaryOps;
};

} // namespace ysmut

#endif
