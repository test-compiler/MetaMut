#ifndef YSMUT_TYPE_DESTRUCTRECORD_H
#define YSMUT_TYPE_DESTRUCTRECORD_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <map>
#include <string>
#include <vector>

#include "Mutator.h"

namespace ysmut {

class DestructRecord : public Mutator,
                       public clang::RecursiveASTVisitor<DestructRecord> {
public:
  using VisitorTy = clang::RecursiveASTVisitor<DestructRecord>;
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitDeclStmt(clang::DeclStmt *DS);
  bool TraverseFunctionDecl(clang::FunctionDecl *FD);
  bool VisitMemberExpr(clang::MemberExpr *ME);

private:
  std::vector<clang::VarDecl *> TheVars;
  std::vector<clang::MemberExpr *> TheMembers;
  std::map<clang::Decl *, clang::DeclStmt *> declToStmt;
};

}  // namespace ysmut

#endif  // YSMUT_TYPE_DESTRUCTRECORD_H
