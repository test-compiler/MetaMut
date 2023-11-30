#ifndef YSMUT_EXPR_DECAYSMALLSTRUCT_H
#define YSMUT_EXPR_DECAYSMALLSTRUCT_H

#include "Mutator.h"
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <map>

namespace ysmut {

class DecaySmallStruct : public Mutator,
                         public clang::RecursiveASTVisitor<DecaySmallStruct> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitMemberExpr(clang::MemberExpr *ME);

private:
  std::vector<clang::VarDecl *> TheVars;
  std::map<clang::VarDecl *, std::vector<clang::MemberExpr *>> VarUses;
};

} // namespace ysmut

#endif
