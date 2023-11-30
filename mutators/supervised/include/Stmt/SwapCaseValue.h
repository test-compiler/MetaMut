#ifndef YSMUT_EXPR_SWAPCASEVALUE_H
#define YSMUT_EXPR_SWAPCASEVALUE_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class SwapCaseValue : public Mutator,
                      public clang::RecursiveASTVisitor<SwapCaseValue> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitSwitchStmt(clang::SwitchStmt *SS);

private:
  std::vector<clang::SwitchStmt *> TheSwitches;
};

} // namespace ysmut

#endif
