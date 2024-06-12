#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class SubstituteWithEquivalentMathOperation
    : public Mutator,
      public clang::RecursiveASTVisitor<SubstituteWithEquivalentMathOperation> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    BinaryOperator *expr = randElement(TheOperators);

    // perform rewrite
    auto newOps = computeEquivalentMathOperation(expr);
    BinaryOperatorKind newOpKind = newOps[randIndex(newOps.size())];
    SourceLocation OpLoc = expr->getOperatorLoc();
    std::string NewOp = BinaryOperator::getOpcodeStr(newOpKind).str();
    getRewriter().ReplaceText(
        OpLoc, expr->getOpcodeStr(expr->getOpcode()).size(), NewOp);

    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator *BO) {
    if (BO->isMultiplicativeOp() || BO->isAdditiveOp())
    {
      TheOperators.push_back(BO);
    }
    return true;
  }

private:
  std::vector<clang::BinaryOperator *> TheOperators;

  std::vector<clang::BinaryOperatorKind> computeEquivalentMathOperation(
      clang::BinaryOperator *op) {
    std::vector<BinaryOperatorKind> equivalentMathOperation;

    switch (op->getOpcode())
    {
    case BO_Add:
        equivalentMathOperation.push_back(BO_Sub);
        break;
    case BO_Sub:
        equivalentMathOperation.push_back(BO_Add);
        break;
    case BO_Mul:
        equivalentMathOperation.push_back(BO_Div);
        break;
    case BO_Div:
        equivalentMathOperation.push_back(BO_Mul);
        break;
    default:
        break;
    }

    return equivalentMathOperation;
  }
};

static RegisterMutator<SubstituteWithEquivalentMathOperation> M("u3.SubstituteWithEquivalentMathOperation", "This mutator selects a binary mathematical operation (like addition, subtraction, multiplication, division) and replaces it with an equivalent operation (like a + b becomes a - (-b) for addition).");
