#ifndef YSMUT_FUNC_REMOVEUNUSEDFUNCTION_H
#define YSMUT_FUNC_REMOVEUNUSEDFUNCTION_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include "Mutator.h"

namespace ysmut {

class RemoveUnusedFunction : public Mutator,
                      public clang::RecursiveASTVisitor<RemoveUnusedFunction> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitCallExpr(clang::CallExpr *CE);

private:
  std::map<clang::FunctionDecl *, bool> TheFuncs;
};

} // namespace ysmut

#endif
