#ifndef YSMUT_EXPR_CHANGEDECLREF_H
#define YSMUT_EXPR_CHANGEDECLREF_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <set>
#include <stack>
#include <string>

#include "Mutator.h"

namespace ysmut {

class ChangeDeclRef : public Mutator,
                      public clang::RecursiveASTVisitor<ChangeDeclRef> {

  using VisitorTy = clang::RecursiveASTVisitor<ChangeDeclRef>;
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);
  bool VisitVarDecl(clang::VarDecl *VD);
  bool TraverseCompoundStmt(clang::CompoundStmt *CS);
  bool TraverseFunctionDecl(clang::FunctionDecl *FD);

private:
  std::map<clang::DeclRefExpr *, std::vector<clang::VarDecl *>> ExprVarsMap;
  std::vector<clang::DeclRefExpr *> TheRefs;
  std::vector<std::vector<clang::VarDecl *>> TheVars;
};

} // namespace ysmut

#endif
