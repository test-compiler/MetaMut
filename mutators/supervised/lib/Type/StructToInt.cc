#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/TypeLoc.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"
#include "Type/StructToInt.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<StructToInt> M(
    "struct-to-int", "Change a struct type to int type.");

bool StructToInt::VisitElaboratedTypeLoc(clang::ElaboratedTypeLoc ETL) {
  if (ETL.getNamedTypeLoc().getType()->isStructureType()) {
    const RecordDecl *RD = ETL.getNamedTypeLoc().getType()->getAsStructureType()->getDecl()->getDefinition();
    StructDecls.push_back(RD->getCanonicalDecl());
    StructsMap[RD->getCanonicalDecl()].push_back(ETL);
  }
  return true;
}

bool StructToInt::mutate() {
  TraverseAST(getASTContext());
  if (StructsMap.empty()) return false;

  auto *selectedStruct = randElement(StructDecls);

  for (auto &loc : StructsMap[selectedStruct]) {
    getRewriter().ReplaceText(loc.getSourceRange(), "int");
  }

  return true;
}
