#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ConvertBooleanToBitwise : public Mutator,
                                 public RecursiveASTVisitor<ConvertBooleanToBitwise> {
public:
  using Mutator::Mutator;

  bool VisitBinaryOperator(BinaryOperator *BO) {
    if (BO->isLogicalOp()) {
      BinaryOperators.push_back(BO);
    }
    return true;
  }

  bool VisitUnaryOperator(UnaryOperator *UO) {
    if (UO->getOpcode() == UO_LNot) {
      UnaryOperators.push_back(UO);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (BinaryOperators.empty() && UnaryOperators.empty()) {
      return false;
    }

    if (randBool()) { 
      if (!UnaryOperators.empty()) {
        UnaryOperator *UO = randElement(UnaryOperators);
        std::string exprText = getSourceText(UO->getSubExpr());
        getRewriter().ReplaceText(UO->getSourceRange(), "~" + exprText);
        return true;
      }
    } else {
      if (!BinaryOperators.empty()) {
        BinaryOperator *BO = randElement(BinaryOperators);
        BinaryOperatorKind BOKind = BO->getOpcode();
        std::string NewOp;
        if (BOKind == BO_LAnd) {
          NewOp = "&";
        } else if (BOKind == BO_LOr) {
          NewOp = "|";
        } else {
          return false;
        }
        SourceLocation OpLoc = BO->getOperatorLoc();
        getRewriter().ReplaceText(OpLoc, BO->getOpcodeStr(BOKind).size(), NewOp);
        return true;
      }
    }
    return false;
  }

private:
  std::vector<UnaryOperator *> UnaryOperators;
  std::vector<BinaryOperator *> BinaryOperators;
};

static RegisterMutator<ConvertBooleanToBitwise> M("u3.ConvertBooleanToBitwise", "Convert Boolean Operations to Bitwise Operations");
