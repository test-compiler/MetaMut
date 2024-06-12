#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class SwitchFloatingPointPrecision
    : public Mutator,
      public clang::RecursiveASTVisitor<SwitchFloatingPointPrecision> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    if (VD->getType()->isFloatingType()) {
      TheVars.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVars.empty()) return false;

    VarDecl *VD = randElement(TheVars);
    QualType VDType = VD->getType();
    std::string NewType;

    if (VDType->isSpecificBuiltinType(BuiltinType::Float)) {
      NewType = "double";
    } else if (VDType->isSpecificBuiltinType(BuiltinType::Double)) {
      NewType = "float";
    } else {
      return false;
    }

    getRewriter().ReplaceText(VD->getTypeSourceInfo()->getTypeLoc().getSourceRange(), NewType);

    return true;
  }

private:
  std::vector<VarDecl*> TheVars;
};

static RegisterMutator<SwitchFloatingPointPrecision> M("u3.SwitchFloatingPointPrecision", "Switch the precision of floating point variable declaration from float to double and vice versa.");
