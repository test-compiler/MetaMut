#ifndef YSMUT_FUNC_SHUFFLEPARAMETERS_H
#define YSMUT_FUNC_SHUFFLEPARAMETERS_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <vector>

#include "Mutator.h"

namespace ysmut {

class ShuffleParameters : public Mutator,
                          public clang::RecursiveASTVisitor<ShuffleParameters> {

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
