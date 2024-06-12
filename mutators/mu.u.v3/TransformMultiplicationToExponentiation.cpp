#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Sema/Sema.h>
#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class TransformMultiplicationToExponentiation: public Mutator,
                              public clang::RecursiveASTVisitor<TransformMultiplicationToExponentiation> {
public:
  using Mutator::Mutator;

  bool VisitBinaryOperator(clang::BinaryOperator *BO) {
    if(BO->getOpcode() == BO_Mul) {
        TheOps.push_back(BO);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOps.empty()) return false;

    BinaryOperator *expr = randElement(TheOps);

    // Replace multiplication operator with a call to pow() function.
    std::string oldText = getSourceText(expr);
    std::string newText = "pow(" + getSourceText(expr->getLHS()) + ", " + getSourceText(expr->getRHS()) + ")";
    
    getRewriter().ReplaceText(expr->getSourceRange(), newText);
    
    return true;
  }

private:
  std::vector<clang::BinaryOperator *> TheOps;
};

static RegisterMutator<TransformMultiplicationToExponentiation> M("u3.TransformMultiplicationToExponentiation", 
    "Change arithmetic operation from multiplication to exponentiation.");
