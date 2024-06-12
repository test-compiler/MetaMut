#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class IncreaseArraySize : public Mutator,
                          public clang::RecursiveASTVisitor<IncreaseArraySize> {

public:
  using Mutator::Mutator;

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getType()->isArrayType()) {
      TheVars.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVars.empty()) return false;
    
    VarDecl *var = randElement(TheVars);
    const ArrayType *type = dyn_cast<ArrayType>(var->getType());
    
    if(!type) return false;
    
    if (!isa<ConstantArrayType>(type)) return false;

    const ConstantArrayType *cat = cast<ConstantArrayType>(type);
    llvm::APInt oldSize = cat->getSize();

    if(oldSize.uge(llvm::APInt(oldSize.getBitWidth(), UINT_MAX))) return false;

    llvm::APInt newSize = oldSize.zext(oldSize.getBitWidth() + 1) * 2; 

    llvm::SmallVector<char, 256> strNewSizeVec;
    newSize.toString(strNewSizeVec, 10, false, false);
    std::string strNewSize(strNewSizeVec.begin(), strNewSizeVec.end());

    llvm::SmallVector<char, 256> strOldSizeVec;
    oldSize.toString(strOldSizeVec, 10, false, false);
    std::string strOldSize(strOldSizeVec.begin(), strOldSizeVec.end());
    
    CharSourceRange oldSizeRange = getRewriter().getSourceMgr().getExpansionRange(
        var->getTypeSourceInfo()->getTypeLoc().getSourceRange());

    SourceLocation newSizeLocation = findStringLocationFrom(
      oldSizeRange.getEnd().getLocWithOffset(1), strOldSize);

    if(!newSizeLocation.isValid()) return false;

    getRewriter().ReplaceText(newSizeLocation, strOldSize.size(), strNewSize);

    return true;
  }

private:
  std::vector<clang::VarDecl *> TheVars;
};

static RegisterMutator<IncreaseArraySize> M("u3.IncreaseArraySize", "Increase array size by a constant factor of 2.");
