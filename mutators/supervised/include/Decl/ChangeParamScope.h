#ifndef YSMUT_DECL_CHANGEPARAMSCOPE_H
#define YSMUT_DECL_CHANGEPARAMSCOPE_H

#include "Mutator.h"
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <map>

namespace ysmut {

class ChangeParamScope : public Mutator,
                         public clang::RecursiveASTVisitor<ChangeParamScope> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DR);

private:
  std::vector<clang::ParmVarDecl *> TheParams;
  std::map<clang::ParmVarDecl *, clang::FunctionDecl *> ParamLocs;
  std::map<clang::ParmVarDecl *, std::vector<clang::DeclRefExpr *>> ParamUses;
};

} // namespace ysmut

#endif
