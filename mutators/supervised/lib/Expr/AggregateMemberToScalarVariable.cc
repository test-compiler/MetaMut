#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Expr/AggregateMemberToScalarVariable.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<AggregateMemberToScalarVariable> M(
    "aggregate-member-to-scalar-variable",
    "Replace accesses to an aggregate member with a "
    "corresponding scalar variable.");

bool AggregateMemberToScalarVariable::
    VisitArraySubscriptExpr(ArraySubscriptExpr *ASE) {
  if (isMutationSite(ASE)) TheArrays.push_back(ASE);
  return true;
}

bool AggregateMemberToScalarVariable::mutate() {
  TraverseAST(getASTContext());
  if (TheArrays.empty()) return false;

  ArraySubscriptExpr *arrayExpr = randElement(TheArrays);

  std::string baseName =
      getSourceText(arrayExpr->getBase()).str();
  std::string indexName =
      getSourceText(arrayExpr->getIdx()).str();

  // The scalar variable to replace the array access.
  std::string scalarVar = generateUniqueName(baseName);

  // Get the declaration of the array variable
  DeclRefExpr *arrayDeclRef = dyn_cast<DeclRefExpr>(
      arrayExpr->getBase()->IgnoreImpCasts());
  VarDecl *arrayDecl =
      dyn_cast<VarDecl>(arrayDeclRef->getDecl());

  // Get the location of the array variable declaration.
  SourceLocation locEnd = getDeclEndLoc(arrayDecl);

  // Get the type of the array variable's elements.
  QualType elementType =
      arrayExpr->getBase()->getType()->getPointeeType();

  // Insert the scalar variable declaration at the location
  // of the array declaration.
  std::string decl = formatAsDecl(elementType, scalarVar) +
                     " = " + baseName + "[" + indexName +
                     "];\n";
  getRewriter().InsertText(locEnd, decl);

  // Replace the array access with the scalar variable.
  getRewriter().ReplaceText(
      arrayExpr->getSourceRange(), scalarVar);

  return true;
}
