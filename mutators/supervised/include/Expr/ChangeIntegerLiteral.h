#ifndef YSMUT_EXPR_CHANGEINTEGERLITERAL_H
#define YSMUT_EXPR_CHANGEINTEGERLITERAL_H

#include "Mutator.h"
#include <clang/AST/RecursiveASTVisitor.h>

namespace ysmut {

class ChangeIntegerLiteral
    : public Mutator,
      public clang::RecursiveASTVisitor<ChangeIntegerLiteral> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitIntegerLiteral(clang::IntegerLiteral *IL);

private:
  std::vector<clang::IntegerLiteral *> TheLiterals;
};

} // namespace ysmut

#endif
