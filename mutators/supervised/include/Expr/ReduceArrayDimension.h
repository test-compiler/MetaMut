#ifndef YSMUT_EXPR_REDUCEARRAYDIMENSION_H
#define YSMUT_EXPR_REDUCEARRAYDIMENSION_H

#include "Mutator.h"
#include <clang/AST/RecursiveASTVisitor.h>

namespace ysmut {

class ReduceArrayDimension
    : public Mutator,
      public clang::RecursiveASTVisitor<ReduceArrayDimension> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE);
  bool VisitVarDecl(clang::VarDecl *VD);

private:
  std::vector<clang::VarDecl *> TheVars;
  std::map<clang::VarDecl *, std::vector<clang::ArraySubscriptExpr *>>
      VarToExprs;
};

} // namespace ysmut

#endif
