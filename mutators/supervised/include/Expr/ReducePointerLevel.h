#ifndef YSMUT_EXPR_REDUCEPOINTERLEVEL_H
#define YSMUT_EXPR_REDUCEPOINTERLEVEL_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <map>

#include "Mutator.h"

namespace ysmut {

class ReducePointerLevel : public Mutator,
                           public clang::RecursiveASTVisitor<ReducePointerLevel> {

  using VisitorTy = clang::RecursiveASTVisitor<ReducePointerLevel>;
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);
  bool TraverseUnaryOperator(clang::UnaryOperator *UO);

private:
  std::vector<clang::VarDecl *> TheVars;
  std::map<clang::VarDecl *, std::vector<clang::DeclRefExpr *>> VarToExprs;
  std::map<clang::VarDecl *, std::vector<clang::UnaryOperator *>> VarToUos;
};

} // namespace ysmut

#endif
