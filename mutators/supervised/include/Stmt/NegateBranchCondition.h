#ifndef YSMUT_STMT_NEGATEBRANCHCONDITION_H
#define YSMUT_STMT_NEGATEBRANCHCONDITION_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class NegateBranchCondition
    : public Mutator,
      public clang::RecursiveASTVisitor<NegateBranchCondition> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitIfStmt(clang::IfStmt *IS);

private:
  std::vector<clang::IfStmt *> TheIfStmts;
};

} // namespace ysmut

#endif
