#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Type.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ChangeArrayToPointerInitialization : public Mutator,
                      public clang::RecursiveASTVisitor<ChangeArrayToPointerInitialization> {

  using VisitorTy = clang::RecursiveASTVisitor<ChangeArrayToPointerInitialization>;

public:
  using Mutator::Mutator;
  
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVars.empty()) return false;

    VarDecl *var = randElement(TheVars);

    const ConstantArrayType* arrayType = dyn_cast<ConstantArrayType>(var->getType().getTypePtr());
    if (!arrayType) return false;

    // replace the type in the variable declaration
    QualType newType = getASTContext().getPointerType(arrayType->getElementType());
    getRewriter().ReplaceText(var->getTypeSourceInfo()->getTypeLoc().getSourceRange(), 
                              formatAsDecl(newType, var->getNameAsString()));

    // Replace the array initialization with malloc
    std::string sizeStr = std::to_string(arrayType->getSize().getZExtValue());
    std::string mallocStr = "malloc(sizeof(" + newType.getAsString() + ") * " + sizeStr + ")";
    getRewriter().ReplaceText(var->getInit()->getSourceRange(), mallocStr);

    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->hasInit() && isa<ConstantArrayType>(VD->getType().getTypePtr())) {
      TheVars.push_back(VD);
    }
    return true;
  }

private:
  std::vector<clang::VarDecl *> TheVars;
};

static RegisterMutator<ChangeArrayToPointerInitialization> M("u3.ChangeArrayToPointerInitialization", "Convert array initialization into pointer initialization with dynamic memory allocation.");
