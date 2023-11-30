#ifndef YSMUT_DECL_REMOVEINITIALIZATION_H
#define YSMUT_DECL_REMOVEINITIALIZATION_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class RemoveInitialization : public Mutator,
                             public clang::RecursiveASTVisitor<RemoveInitialization> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);

private:
  std::vector<clang::VarDecl *> TheVars;
};

} // namespace ysmut

#endif
