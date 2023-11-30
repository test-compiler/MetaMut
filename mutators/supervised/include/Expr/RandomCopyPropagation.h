#ifndef YSMUT_EXPR_RANDOMCOPYPROPAGATION_H
#define YSMUT_EXPR_RANDOMCOPYPROPAGATION_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class RandomCopyPropagation
    : public Mutator,
      public clang::RecursiveASTVisitor<RandomCopyPropagation> {
  using VisitorTy = clang::RecursiveASTVisitor<RandomCopyPropagation>;

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool TraverseBinaryOperator(clang::BinaryOperator *BO);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DR);

private:
  std::map<clang::VarDecl *, std::vector<clang::Expr *>> choices;
  std::map<clang::VarDecl *, std::vector<clang::DeclRefExpr *>> refExprs;
};

} // namespace ysmut

#endif
