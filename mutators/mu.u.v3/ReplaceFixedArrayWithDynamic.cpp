#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>
#include <sstream>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceFixedArrayWithDynamic : public Mutator,
                                     public clang::RecursiveASTVisitor<ReplaceFixedArrayWithDynamic> {

public:
  using Mutator::Mutator;

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->hasInit() && VD->getType()->isConstantArrayType()) {
      TheArrays.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheArrays.empty()) return false;

    VarDecl *VD = randElement(TheArrays);
    auto *ArrayType = dyn_cast<ConstantArrayType>(VD->getType().getTypePtr());
    QualType ElementType = ArrayType->getElementType();
    std::string VarName = VD->getNameAsString();
    std::string ElementTypeStr = ElementType.getAsString();

    std::stringstream NewDeclaration;
    NewDeclaration << ElementTypeStr << "* " << VarName << " = malloc(" << ArrayType->getSize().getZExtValue() << " * sizeof(" << ElementTypeStr << "));";
    getRewriter().ReplaceText(VD->getSourceRange(), NewDeclaration.str());

    return true;
  }

private:
  std::vector<clang::VarDecl *> TheArrays;
};

static RegisterMutator<ReplaceFixedArrayWithDynamic> M("u3.ReplaceFixedArrayWithDynamic", 
    "Replaces fixed array with dynamically allocated memory using malloc");
