#ifndef YSMUT_EXPR_FLATTENARRAY_H
#define YSMUT_EXPR_FLATTENARRAY_H

#include "Mutator.h"
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <map>

namespace ysmut {

class FlattenArray : public Mutator,
                     public clang::RecursiveASTVisitor<FlattenArray> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE);

private:
  std::vector<clang::VarDecl *> TheVars;
  std::map<clang::VarDecl *, std::vector<clang::ArraySubscriptExpr *>> VarUses;
  std::map<clang::VarDecl *, unsigned long long> ArraySizes;
};

} // namespace ysmut

#endif
