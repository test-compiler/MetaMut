#ifndef YSMUT_DECL_ADDINITIALIZATION_H
#define YSMUT_DECL_ADDINITIALIZATION_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class AddInitialization : public Mutator,
                          public clang::RecursiveASTVisitor<AddInitialization> {

  using VisitorTy = clang::RecursiveASTVisitor<AddInitialization>;
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool TraverseFunctionDecl(clang::FunctionDecl *FD);
  bool VisitVarDecl(clang::VarDecl *VD);

private:
  std::vector<clang::VarDecl *> TheVars;
};

} // namespace ysmut

#endif
