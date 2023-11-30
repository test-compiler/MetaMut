#ifndef YSMUT_EXPR_INCREASEARRAYDIMENSION_H
#define YSMUT_EXPR_INCREASEARRAYDIMENSION_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <map>
#include <string>

#include "Mutator.h"

namespace ysmut {

class IncreaseArrayDimension
    : public Mutator,
      public clang::RecursiveASTVisitor<IncreaseArrayDimension> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE);

private:
  std::vector<clang::VarDecl *> TheVars;
  std::map<clang::VarDecl *, std::vector<clang::ArraySubscriptExpr *>>
      VarToExprs;
};

} // namespace ysmut

#endif
