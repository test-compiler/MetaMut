#ifndef YSMUT_STMT_SWITCHTOIF_H
#define YSMUT_STMT_SWITCHTOIF_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

#include "Mutator.h"

namespace ysmut {

class SwitchToIf : public Mutator,
                   public clang::RecursiveASTVisitor<SwitchToIf> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitSwitchStmt(clang::SwitchStmt *SS);

private:
  std::vector<clang::SwitchStmt *> TheSwitches;
};

} // namespace ysmut

#endif
