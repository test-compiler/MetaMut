#ifndef YSMUT_EXPR_INCREASEPOINTERLEVEL_H
#define YSMUT_EXPR_INCREASEPOINTERLEVEL_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <map>
#include <vector>

#include "Mutator.h"

namespace ysmut {

class IncreasePointerLevel : public Mutator,
                             public clang::RecursiveASTVisitor<IncreasePointerLevel> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);

private:
  std::vector<clang::VarDecl *> TheVars;
  std::map<clang::VarDecl *, std::vector<clang::DeclRefExpr *>> VarToRefs;
};

} // namespace ysmut

#endif
