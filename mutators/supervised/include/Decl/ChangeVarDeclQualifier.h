#ifndef YSMUT_DECL_CHANGEVARDECLQUALIFIER_H
#define YSMUT_DECL_CHANGEVARDECLQUALIFIER_H

#include "Mutator.h"
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <map>

namespace ysmut {

class ChangeVarDeclQualifier
    : public Mutator,
      public clang::RecursiveASTVisitor<ChangeVarDeclQualifier> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);

private:
  std::vector<clang::VarDecl *> TheVars;
};

} // namespace ysmut

#endif
