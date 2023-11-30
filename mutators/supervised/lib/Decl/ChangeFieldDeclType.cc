#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Decl/ChangeFieldDeclType.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<ChangeFieldDeclType> M(
    "change-fielddecltype", "Change a FieldDecl's type to a compliant one.");

bool ChangeFieldDeclType::VisitFieldDecl(FieldDecl *FD) {
  if (isMutationSite(FD)) TheFields.push_back(FD);
  return true;
}

bool ChangeFieldDeclType::VisitDecl(Decl *D) {
  if (ValueDecl *V = dyn_cast<ValueDecl>(D))
    TheTypes.insert(V->getType());
  return true;
}

bool ChangeFieldDeclType::mutate() {
  TraverseAST(getASTContext());
  if (TheFields.empty()) return false;

  FieldDecl *oldField = randElement(TheFields);
  QualType oldType = oldField->getType();

  // Collect all compatible types
  std::vector<QualType> compatibleTypes;
  for (const auto &type : TheTypes) {
    if (type == oldType) continue;
    if (getCompilerInstance().getSema().CheckAssignmentConstraints(
            oldField->getLocation(), type, oldType) ==
        Sema::AssignConvertType::Compatible) {
      compatibleTypes.push_back(type);
    }
  }

  if (compatibleTypes.empty()) return false; // no suitable replacement found

  // Randomly select a new type
  QualType newType = randElement(compatibleTypes);

  // Replace the old type with the new one
  getRewriter().ReplaceText(oldField->getTypeSourceInfo()->getTypeLoc().getSourceRange(), newType.getAsString());

  return true;
}
