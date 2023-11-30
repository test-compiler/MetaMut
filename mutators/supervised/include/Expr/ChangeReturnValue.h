#ifndef YSMUT_EXPR_CHANGERETURNVALUE_H
#define YSMUT_EXPR_CHANGERETURNVALUE_H

#include "Mutator.h"
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <map>

namespace ysmut {

class ChangeReturnValue
    : public Mutator,
      public clang::RecursiveASTVisitor<ChangeReturnValue> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitReturnStmt(clang::ReturnStmt *RS);

private:
  std::vector<clang::ReturnStmt *> TheReturns;
};

} // namespace ysmut

#endif
