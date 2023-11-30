#ifndef YSMUT_EXPR_CHANGEFLOATLITERAL_H
#define YSMUT_EXPR_CHANGEFLOATLITERAL_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class ChangeFloatLiteral : public Mutator,
                           public clang::RecursiveASTVisitor<ChangeFloatLiteral> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFloatingLiteral(clang::FloatingLiteral *FL);

private:
  std::vector<clang::FloatingLiteral *> TheLiterals;
};

} // namespace ysmut

#endif
