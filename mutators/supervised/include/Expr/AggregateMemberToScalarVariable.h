#ifndef YSMUT_EXPR_AGGREGATEMEMBERTOSCALARVARIABLE_H
#define YSMUT_EXPR_AGGREGATEMEMBERTOSCALARVARIABLE_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

#include "Mutator.h"

namespace ysmut {

class AggregateMemberToScalarVariable : public Mutator,
                                        public clang::RecursiveASTVisitor<AggregateMemberToScalarVariable> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE);

private:
  std::vector<clang::ArraySubscriptExpr *> TheArrays;
};

} // namespace ysmut

#endif
