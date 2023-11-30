#ifndef YSMUT_STMT_IFTOSWITCH_H
#define YSMUT_STMT_IFTOSWITCH_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

#include "Mutator.h"

namespace ysmut {

class IfToSwitch : public Mutator,
                   public clang::RecursiveASTVisitor<IfToSwitch> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitIfStmt(clang::IfStmt *IS);

private:
  std::vector<clang::IfStmt *> TheIfs;
};

} // namespace ysmut

#endif
