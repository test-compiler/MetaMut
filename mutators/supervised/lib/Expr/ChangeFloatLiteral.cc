#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Expr/ChangeFloatLiteral.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<ChangeFloatLiteral> M(
    "change-floatliteral", "Change a FloatingLiteral's value.");

bool ChangeFloatLiteral::VisitFloatingLiteral(FloatingLiteral *FL) {
  if (isMutationSite(FL)) TheLiterals.push_back(FL);
  return true;
}

bool ChangeFloatLiteral::mutate() {
  TraverseAST(getASTContext());
  if (TheLiterals.empty()) return false;

  FloatingLiteral *literal = randElement(TheLiterals);

  // Generate a new random float
  float newFloat = getManager().randreal(INT_MIN, INT_MAX);

  // Replace the old literal with the new one
  getRewriter().ReplaceText(
      literal->getSourceRange(), std::to_string(newFloat));

  return true;
}
