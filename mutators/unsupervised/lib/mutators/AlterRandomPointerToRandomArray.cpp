#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class AlterRandomPointerToRandomArray : public Mutator,
                      public clang::RecursiveASTVisitor<AlterRandomPointerToRandomArray> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (ThePointers.empty()) return false;

    VarDecl *pointer = randElement(ThePointers);
    QualType pointerType = pointer->getType();
    if(!pointerType->isPointerType()) return false;

    // Change pointer to array
    unsigned arraySize = randIndex(10) + 1;  // Random size between 1 and 10
    QualType arrayType = getASTContext().getConstantArrayType(pointerType->getPointeeType(), 
                                  llvm::APInt(32, arraySize), nullptr, ArrayType::Normal, 0);
    getRewriter().ReplaceText(pointer->getSourceRange(), formatAsDecl(arrayType, pointer->getNameAsString()));

    // Change dereference to array indexing
    for(auto *unaryOperator : TheDereferences) {
      if(unaryOperator->getOperatorLoc() == pointer->getLocation()) {
        getRewriter().ReplaceText(unaryOperator->getSourceRange(), 
                                  pointer->getNameAsString() + "[0]");  // Assume index 0 for simplicity
      }
    }

    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    if(VD->getType()->isPointerType()) {
      ThePointers.push_back(VD);
    }
    return true;
  }

  bool VisitUnaryOperator(clang::UnaryOperator *UO) {
    if(UO->getOpcode() == UO_Deref) {
      TheDereferences.push_back(UO);
    }
    return true;
  }

private:
  std::vector<clang::VarDecl *> ThePointers;
  std::vector<clang::UnaryOperator *> TheDereferences;
};

static RegisterMutator<AlterRandomPointerToRandomArray> M(
    "AlterRandomPointerToRandomArray", "Change a pointer declaration to an array declaration and modify dereference to array indexing.");