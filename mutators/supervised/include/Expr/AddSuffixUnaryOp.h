#ifndef YSMUT_EXPR_ADDSUFFIXUNARYOP_H
#define YSMUT_EXPR_ADDSUFFIXUNARYOP_H

#include "Mutator.h"
#include <clang/AST/RecursiveASTVisitor.h>

namespace ysmut {

class AddSuffixUnaryOp : public Mutator,
                         public clang::RecursiveASTVisitor<AddSuffixUnaryOp> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);

private:
  std::vector<clang::DeclRefExpr *> TheRefs;
};

} // namespace ysmut

#endif
