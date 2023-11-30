#ifndef YSMUT_FUNC_SIMPLEINLINER_H
#define YSMUT_FUNC_SIMPLEINLINER_H

#include "Mutator.h"
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

namespace ysmut {

class SimpleInliner
    : public Mutator,
      public clang::RecursiveASTVisitor<SimpleInliner> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitCallExpr(clang::CallExpr *CE);
  bool TraverseCompoundStmt(clang::CompoundStmt *CS);

private:
  std::vector<std::pair<const clang::CallExpr *, clang::Stmt *>> TheCalls;
  clang::Stmt *CurrentParent = nullptr;
};

} // namespace ysmut

#endif
