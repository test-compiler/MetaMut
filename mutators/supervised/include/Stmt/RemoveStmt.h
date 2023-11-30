#ifndef YSMUT_STMT_REMOVESTMT_H
#define YSMUT_STMT_REMOVESTMT_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class RemoveStmt : public Mutator,
                   public clang::RecursiveASTVisitor<RemoveStmt> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitStmt(clang::Stmt *S);

private:
  std::vector<clang::Stmt *> TheStatements;
};

} // namespace ysmut

#endif
