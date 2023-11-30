#ifndef YSMUT_FUNC_REMOVEUNUSEDPARAMETER_H
#define YSMUT_FUNC_REMOVEUNUSEDPARAMETER_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <map>
#include <string>
#include <vector>

#include "Mutator.h"

namespace ysmut {

class RemoveUnusedParameter
    : public Mutator,
      public clang::RecursiveASTVisitor<RemoveUnusedParameter> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitCallExpr(clang::CallExpr *CE);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);

private:
  std::vector<clang::FunctionDecl *> TheFuncs;
  std::map<clang::FunctionDecl *, std::vector<clang::CallExpr *>> FuncToCalls;
  std::map<clang::ParmVarDecl *, int> ParamUsageCount;
};

} // namespace ysmut

#endif
