#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ChangeVarDeclToAuto
    : public Mutator,
      public clang::RecursiveASTVisitor<ChangeVarDeclToAuto> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVarDecls.empty()) return false;

    VarDecl *decl = randElement(TheVarDecls);

    // ensure the variable declaration has an initializer
    if (!decl->hasInit()) return false;

    // perform rewrite
    SourceRange typeRange = decl->getTypeSourceInfo()->getTypeLoc().getSourceRange();
    getRewriter().ReplaceText(typeRange, "auto");

    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    TheVarDecls.push_back(VD);
    return true;
  }

private:
  std::vector<clang::VarDecl *> TheVarDecls;
};

static RegisterMutator<ChangeVarDeclToAuto> M("u3.ChangeVarDeclToAuto", "Change variable declaration with an explicit type specifier, to 'auto'.");
