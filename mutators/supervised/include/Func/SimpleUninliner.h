#ifndef YSMUT_FUNC_SIMPLEUNINLINER_H
#define YSMUT_FUNC_SIMPLEUNINLINER_H

#include "Mutator.h"
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

namespace ysmut {

class SimpleUninliner
    : public Mutator,
      public clang::RecursiveASTVisitor<SimpleUninliner> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitCompoundStmt(clang::CompoundStmt *CS);

private:
  std::vector<clang::CompoundStmt *> TheStmts;
  clang::FunctionDecl *FirstFD = nullptr;
};

} // namespace ysmut

#endif
