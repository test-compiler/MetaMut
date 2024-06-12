#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class TransformAssignmentToOperator
    : public Mutator,
      public clang::RecursiveASTVisitor<TransformAssignmentToOperator> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    BinaryOperator *expr = randElement(TheOperators);

    // perform rewrite
    auto newOps = computeValidAlternatives(expr);
    BinaryOperatorKind newOpKind = newOps[randIndex(newOps.size())];
    SourceLocation OpLoc = expr->getOperatorLoc();
    std::string NewOp = BinaryOperator::getOpcodeStr(newOpKind).str();
    getRewriter().ReplaceText(
        OpLoc, expr->getOpcodeStr(expr->getOpcode()).size(), NewOp);

    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator *BO) {
    if (BO->isAssignmentOp() && !BO->isCompoundAssignmentOp()) {
        Expr *lhs = BO->getLHS(), *rhs = BO->getRHS();
        if (isa<DeclRefExpr>(lhs->IgnoreImpCasts()) && isa<BinaryOperator>(rhs)) {
            BinaryOperator *rhsOp = cast<BinaryOperator>(rhs);
            if (isa<DeclRefExpr>(rhsOp->getLHS()->IgnoreParenImpCasts()) ||
                isa<DeclRefExpr>(rhsOp->getRHS()->IgnoreParenImpCasts())) {
                    TheOperators.push_back(BO);
            }
        }
    }
    return true;
  }

private:
  std::vector<clang::BinaryOperator *> TheOperators;

  std::vector<clang::BinaryOperatorKind> computeValidAlternatives(
      clang::BinaryOperator *op) {
    std::vector<BinaryOperatorKind> validAlternatives;

    BinaryOperator *rhsOp = cast<BinaryOperator>(op->getRHS());
    switch(rhsOp->getOpcode()) {
        case BO_Add: validAlternatives.push_back(BO_AddAssign); break;
        case BO_Sub: validAlternatives.push_back(BO_SubAssign); break;
        case BO_Mul: validAlternatives.push_back(BO_MulAssign); break;
        case BO_Div: validAlternatives.push_back(BO_DivAssign); break;
        case BO_Rem: validAlternatives.push_back(BO_RemAssign); break;
        case BO_Shl: validAlternatives.push_back(BO_ShlAssign); break;
        case BO_Shr: validAlternatives.push_back(BO_ShrAssign); break;
        case BO_And: validAlternatives.push_back(BO_AndAssign); break;
        case BO_Or : validAlternatives.push_back(BO_OrAssign);  break;
        case BO_Xor: validAlternatives.push_back(BO_XorAssign); break;
        default: break;
    }

    return validAlternatives;
  }
};

static RegisterMutator<TransformAssignmentToOperator> M("u3.TransformAssignmentToOperator", "Transform simple assignment operation to compound assignment operation.");
