#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Func/ChangeParmType.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<ChangeParmType> M(
    "change-parmtype", "Change a ParmVarDecl's type to a compliant one.");

bool ChangeParmType::VisitParmVarDecl(ParmVarDecl *PVD) {
  if (isMutationSite(PVD)) TheParms.push_back(PVD);
  return true;
}

bool ChangeParmType::VisitDecl(Decl *D) {
  if (ValueDecl *V = dyn_cast<ValueDecl>(D))
    TheTypes.insert(V->getType());
  return true;
}

bool ChangeParmType::mutate() {
  TraverseAST(getASTContext());
  if (TheParms.empty()) return false;

  ParmVarDecl *oldParm = randElement(TheParms);
  QualType oldType = oldParm->getType();

  // Collect all compatible types
  std::vector<QualType> compatibleTypes;
  for (const auto &type : TheTypes) {
    if (type == oldType) continue;
    if (getCompilerInstance().getSema().CheckAssignmentConstraints(
            oldParm->getLocation(), type, oldType) ==
        Sema::AssignConvertType::Compatible) {
      compatibleTypes.push_back(type);
    }
  }

  if (compatibleTypes.empty()) return false; // no suitable replacement found

  // Randomly select a new type
  QualType newType = randElement(compatibleTypes);

  // Replace the old type with the new one
  getRewriter().ReplaceText(oldParm->getTypeSourceInfo()->getTypeLoc().getSourceRange(), newType.getAsString());

  return true;
}
