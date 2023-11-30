#ifndef YSMUT_STMT_DUPLICATESTATEMENT_H
#define YSMUT_STMT_DUPLICATESTATEMENT_H

#include "Mutator.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <vector>

namespace ysmut {

class DuplicateStatement
    : public Mutator,
      public clang::RecursiveASTVisitor<DuplicateStatement> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitCompoundStmt(clang::CompoundStmt *S);

private:
  std::vector<clang::Stmt *> Stmts;
};

} // namespace ysmut

#endif
