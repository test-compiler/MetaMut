#ifndef YSMUT_EXPR_CHANGEVARDECLSCOPE_H
#define YSMUT_EXPR_CHANGEVARDECLSCOPE_H

#include "Mutator.h"
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <map>

namespace ysmut {

class ChangeVarDeclScope
    : public Mutator,
      public clang::RecursiveASTVisitor<ChangeVarDeclScope> {
  using VisitorTy = clang::RecursiveASTVisitor<ChangeVarDeclScope>;
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DR);
  bool TraverseFunctionDecl(clang::FunctionDecl *FD);

private:
  clang::FunctionDecl *CurrFD = nullptr;
  std::vector<clang::VarDecl *> TheVars;
  std::map<clang::VarDecl *, clang::FunctionDecl *> VarLocs;
  std::map<clang::DeclRefExpr *, clang::FunctionDecl *> DRLocs;
  std::map<clang::VarDecl *, std::vector<clang::DeclRefExpr *>> VarUses;
};

} // namespace ysmut

#endif
