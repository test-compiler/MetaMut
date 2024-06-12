#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceFloatWithDoublePrecision : public Mutator,
                                        public RecursiveASTVisitor<ReplaceFloatWithDoublePrecision> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    auto type = VD->getType().getUnqualifiedType();
    if (type->isFloatingType() && type->isSpecificBuiltinType(BuiltinType::Float)) {
      TheVars.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheVars.empty()) 
      return false;

    VarDecl *vd = randElement(TheVars);
    getRewriter().ReplaceText(vd->getTypeSourceInfo()->getTypeLoc().getSourceRange(), "double");
    return true;
  }

private:
  std::vector<VarDecl*> TheVars;
};

static RegisterMutator<ReplaceFloatWithDoublePrecision> M("u3.ReplaceFloatWithDoublePrecision", "Changes float variables to double for increased precision");
