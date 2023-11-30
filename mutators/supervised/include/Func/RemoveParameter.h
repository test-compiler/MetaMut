#ifndef YSMUT_FUNC_REMOVEPARAMETER_H
#define YSMUT_FUNC_REMOVEPARAMETER_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <map>
#include <string>
#include <vector>

#include "Mutator.h"

namespace ysmut {

class RemoveParameter
    : public Mutator,
      public clang::RecursiveASTVisitor<RemoveParameter> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitCallExpr(clang::CallExpr *CE);

private:
  std::vector<clang::FunctionDecl *> TheFuncs;
  std::map<clang::FunctionDecl *, std::vector<clang::CallExpr *>> FuncToCalls;
};

} // namespace ysmut

#endif
