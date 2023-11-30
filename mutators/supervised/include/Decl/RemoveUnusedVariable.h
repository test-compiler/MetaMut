#ifndef YSMUT_EXPR_REMOVEUNUSEDVARIABLE_H
#define YSMUT_EXPR_REMOVEUNUSEDVARIABLE_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include "Mutator.h"

namespace ysmut {

class RemoveUnusedVariable : public Mutator,
                      public clang::RecursiveASTVisitor<RemoveUnusedVariable> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);

private:
  std::map<clang::VarDecl *, bool> TheVars;
};

} // namespace ysmut

#endif
