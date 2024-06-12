#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class NegateConditionalOperator : public Mutator,
                                  public clang::RecursiveASTVisitor<NegateConditionalOperator> {
public:
  using Mutator::Mutator;

  bool VisitConditionalOperator(ConditionalOperator* CO) {
    TheConditionalOperators.push_back(CO);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheConditionalOperators.empty()) return false;

    ConditionalOperator* conditionalOperator = randElement(TheConditionalOperators);
    SourceRange trueExprRange = conditionalOperator->getTrueExpr()->getSourceRange();
    SourceRange falseExprRange = conditionalOperator->getFalseExpr()->getSourceRange();

    std::string trueExprStr = Lexer::getSourceText(CharSourceRange::getTokenRange(trueExprRange),
                                                   getASTContext().getSourceManager(), getASTContext().getLangOpts()).str();

    std::string falseExprStr = Lexer::getSourceText(CharSourceRange::getTokenRange(falseExprRange),
                                                    getASTContext().getSourceManager(), getASTContext().getLangOpts()).str();

    getRewriter().ReplaceText(trueExprRange, falseExprStr);
    getRewriter().ReplaceText(falseExprRange, trueExprStr);

    return true;
  }

private:
  std::vector<ConditionalOperator*> TheConditionalOperators;
};

static RegisterMutator<NegateConditionalOperator> M("u3.NegateConditionalOperator", "Swap the true and false expressions of a ternary conditional operator.");
