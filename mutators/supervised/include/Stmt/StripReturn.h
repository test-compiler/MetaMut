#ifndef YSMUT_STMT_STRIPRETURN_H
#define YSMUT_STMT_STRIPRETURN_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

#include "Mutator.h"

namespace ysmut {

class StripReturn : public Mutator,
                    public clang::RecursiveASTVisitor<StripReturn> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitReturnStmt(clang::ReturnStmt *RS);

private:
  std::vector<clang::FunctionDecl *> TheFuncs;
  std::vector<clang::ReturnStmt *> TheReturns;
};

} // namespace ysmut

#endif
