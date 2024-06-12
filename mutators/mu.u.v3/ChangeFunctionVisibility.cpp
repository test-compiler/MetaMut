#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ChangeFunctionVisibility
    : public Mutator,
      public clang::RecursiveASTVisitor<ChangeFunctionVisibility> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFunctions.empty()) return false;

    FunctionDecl *fdecl = randElement(TheFunctions);

    if (fdecl->isStatic()) {
      getRewriter().ReplaceText(fdecl->getInnerLocStart(), 6, "");
      return true;
    } else {
      getRewriter().InsertTextBefore(fdecl->getInnerLocStart(), "static ");
      return true;
    }
  }

  bool VisitFunctionDecl(clang::FunctionDecl *F) {
    if (!F->isThisDeclarationADefinition()) return true;
    TheFunctions.push_back(F);
    return true;
  }

private:
  std::vector<clang::FunctionDecl *> TheFunctions;
};

static RegisterMutator<ChangeFunctionVisibility> M("u3.ChangeFunctionVisibility",
    "Changes the visibility of a selected function from public to private, or vice versa.");
