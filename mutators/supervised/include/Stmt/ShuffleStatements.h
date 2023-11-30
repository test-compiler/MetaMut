#ifndef YSMUT_STMT_SHUFFLESTATEMENTS_H
#define YSMUT_STMT_SHUFFLESTATEMENTS_H

#include "Mutator.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <vector>

namespace ysmut {

class ShuffleStatements : public Mutator,
                          public clang::RecursiveASTVisitor<ShuffleStatements> {
  using VisitorTy = clang::RecursiveASTVisitor<ShuffleStatements>;
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitCompoundStmt(clang::CompoundStmt *S);

private:
  std::vector<std::vector<clang::Stmt *>> StmtGroups;
  std::vector<clang::Stmt *> CurrentGroup;
};

} // namespace ysmut

#endif
