#ifndef YSMUT_STMT_SHUFFLEEXPRSTMTS_H
#define YSMUT_STMT_SHUFFLEEXPRSTMTS_H

#include "Mutator.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <vector>

namespace ysmut {

class ShuffleExprStmts : public Mutator,
                         public clang::RecursiveASTVisitor<ShuffleExprStmts> {
  using VisitorTy = clang::RecursiveASTVisitor<ShuffleExprStmts>;

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
