#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class TransformPostfixToPrefixInLoop : public Mutator, public clang::RecursiveASTVisitor<TransformPostfixToPrefixInLoop> {
public:
  using Mutator::Mutator;

  bool VisitUnaryOperator(clang::UnaryOperator *UO) {
    if (UO->isPostfix() && (UO->getOpcode() == UO_PostInc || UO->getOpcode() == UO_PostDec)) {
      TheOperators.push_back(UO);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    UnaryOperator *expr = randElement(TheOperators);
    
    UnaryOperatorKind newOpKind;
    if(expr->getOpcode() == UO_PostInc)
      newOpKind = UO_PreInc;
    else
      newOpKind = UO_PreDec;

    Expr *subExpr = expr->getSubExpr();
    std::string exprStr = " " + cast<DeclRefExpr>(subExpr)->getNameInfo().getAsString() + UnaryOperator::getOpcodeStr(newOpKind).str();

    getRewriter().ReplaceText(expr->getSourceRange(), exprStr);

    return true;
  }

private:
  std::vector<clang::UnaryOperator *> TheOperators;
};

static RegisterMutator<TransformPostfixToPrefixInLoop> M("u3.TransformPostfixToPrefixInLoop", "Transform postfix increment/decrement to prefix form in loops.");
