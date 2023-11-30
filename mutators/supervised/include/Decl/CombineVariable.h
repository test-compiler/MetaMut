#ifndef YSMUT_DECL_COMBINEVARIABLE_H
#define YSMUT_DECL_COMBINEVARIABLE_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <map>
#include <string>
#include <vector>

#include "Mutator.h"

namespace ysmut {

class CombineVariable : public Mutator,
                        public clang::RecursiveASTVisitor<CombineVariable> {
public:
  using VisitorTy = clang::RecursiveASTVisitor<CombineVariable>;
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);
  
private:
  clang::SourceLocation firstFunctionLoc;
  std::vector<clang::VarDecl *> TheVars;
  std::map<clang::VarDecl *, std::vector<clang::DeclRefExpr *>> varDeclToRefs;
};

}  // namespace ysmut

#endif  // YSMUT_TYPE_COMBINEVARIABLE_H
