#ifndef YSMUT_STMT_SWITCHTOGOTO_H
#define YSMUT_STMT_SWITCHTOGOTO_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

#include "Mutator.h"

namespace ysmut {

class SwitchToGoto : public Mutator,
                     public clang::RecursiveASTVisitor<SwitchToGoto> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitSwitchStmt(clang::SwitchStmt *SS);

private:
  std::vector<clang::SwitchStmt *> TheSwitches;
  std::string getSourceTextRemovingBreak(
      clang::Stmt *stmt, const std::string &endLabel);
};

} // namespace ysmut

#endif
