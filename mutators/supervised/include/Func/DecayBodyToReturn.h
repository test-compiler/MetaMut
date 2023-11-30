#ifndef YSMUT_EXPR_DECAYBODYTORETURN_H
#define YSMUT_EXPR_DECAYBODYTORETURN_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class DecayBodyToReturn : public Mutator,
                          public clang::RecursiveASTVisitor<DecayBodyToReturn> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFunctionDecl(clang::FunctionDecl *FD);

private:
  std::vector<clang::FunctionDecl *> TheFuncs;

  void handleStmt(clang::Stmt *S);
};

} // namespace ysmut

#endif
