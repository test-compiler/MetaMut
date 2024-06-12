#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ConvertBoolToInt : public Mutator,
                         public clang::RecursiveASTVisitor<ConvertBoolToInt> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    if(VD->getType().getCanonicalType()->isBooleanType()){
      TheVars.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if(TheVars.empty()) return false;

    VarDecl *selectedVar = randElement(TheVars);

    auto VarTypeRange = selectedVar->getTypeSourceInfo()->getTypeLoc().getSourceRange();
    getRewriter().ReplaceText(VarTypeRange, "int");
    return true;
  }

private:
  std::vector<clang::VarDecl *> TheVars;
};

static RegisterMutator<ConvertBoolToInt> M("u3.ConvertBoolToInt", "Change boolean variables to integer variables");
