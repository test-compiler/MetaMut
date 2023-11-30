#ifndef YSMUT_STMT_GROUPSTMTINTOCOMPOUND_H
#define YSMUT_STMT_GROUPSTMTINTOCOMPOUND_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class GroupStmtsIntoCompound : public Mutator,
                               public clang::RecursiveASTVisitor<GroupStmtsIntoCompound> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitCompoundStmt(clang::CompoundStmt *CS);

private:
  std::vector<clang::CompoundStmt *> TheCompStmts;
};

} // namespace ysmut

#endif
