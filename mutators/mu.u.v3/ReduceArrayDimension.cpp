#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReduceArrayDimension
    : public Mutator,
      public clang::RecursiveASTVisitor<ReduceArrayDimension> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    auto type = VD->getType();
    if (type->isConstantArrayType() || type->isVariableArrayType() || type->isIncompleteArrayType()) {
      const ArrayType *arrayType = dyn_cast<ArrayType>(type.getCanonicalType());
      auto elementType = arrayType->getElementType();
      if(elementType->isConstantArrayType() || elementType->isVariableArrayType() || elementType->isIncompleteArrayType()) {
        Arrays.push_back(VD);
      }
    }
    return true;
  }

  bool VisitArraySubscriptExpr(ArraySubscriptExpr *ASE) {
    Expr *base = ASE->getBase();
    if(auto DRE = dyn_cast<DeclRefExpr>(base->IgnoreParenImpCasts())) {
      if (auto VD = dyn_cast<VarDecl>(DRE->getDecl())) {
        if (std::find(Arrays.begin(), Arrays.end(), VD) != Arrays.end()) {
          std::string newExpr = getSourceText(ASE->getBase());
          for(int i=0; i < NumDims-1; i++)
            newExpr += "[" + getSourceText(ASE->getIdx()) + "]";
          getRewriter().ReplaceText(ASE->getSourceRange(), newExpr);
        }
      }
    }
    return true;
  }

  std::string flattenInitList(InitListExpr *ILE) {
    std::string flattenedList;
    for (int i = 0; i < ILE->getNumInits(); i++) {
      if (!flattenedList.empty())
        flattenedList += ", ";
      if (InitListExpr *subList = dyn_cast<InitListExpr>(ILE->getInit(i))) {
        flattenedList += flattenInitList(subList);
      } else {
        flattenedList += getSourceText(ILE->getInit(i));
      }
    }
    return flattenedList;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if(Arrays.empty())
      return false;

    VarDecl* target = randElement(Arrays);
    
    auto type = target->getType();
    const ArrayType *arrayType = dyn_cast<ArrayType>(type.getCanonicalType());
    auto elementType = arrayType->getElementType();

    NumDims = 2; // Since we only process arrays with 2 or more dimensions

    std::string targetName = target->getNameAsString();
    std::string elementTypeStr = formatAsDecl(elementType, targetName);

    std::string newDeclStr = elementTypeStr;
    if(target->hasInit()) {
      InitListExpr *ILE = dyn_cast<InitListExpr>(target->getInit());
      std::string flattenedList = flattenInitList(ILE);
      newDeclStr += " = {" + flattenedList + "}";
    }

    getRewriter().ReplaceText(target->getSourceRange(), newDeclStr);

    return true;
  }

private:
  std::vector<VarDecl*> Arrays;
  int NumDims;
};

static RegisterMutator<ReduceArrayDimension> M("u3.ReduceArrayDimension", "Reduces an array's dimension by one");
