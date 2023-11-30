#ifndef YSMUT_EXPR_COPYPROPAGATION_H
#define YSMUT_EXPR_COPYPROPAGATION_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class CopyPropagation : public Mutator,
                        public clang::RecursiveASTVisitor<CopyPropagation> {
  using VisitorTy = clang::RecursiveASTVisitor<CopyPropagation>;

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
