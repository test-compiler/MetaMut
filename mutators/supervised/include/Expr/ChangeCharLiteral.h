#ifndef YSMUT_EXPR_CHANGECHARLITERAL_H
#define YSMUT_EXPR_CHANGECHARLITERAL_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class ChangeCharLiteral : public Mutator,
                          public clang::RecursiveASTVisitor<ChangeCharLiteral> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitCharacterLiteral(clang::CharacterLiteral *CL);

private:
  std::vector<clang::CharacterLiteral *> TheLiterals;
};

} // namespace ysmut

#endif
