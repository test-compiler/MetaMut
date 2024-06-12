#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReverseArrayElements : public Mutator,
                             public clang::RecursiveASTVisitor<ReverseArrayElements> {
public:
  using Mutator::Mutator;

  bool VisitInitListExpr(InitListExpr *ILE) {
    TheExprs.push_back(ILE);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheExprs.empty()) return false;

    InitListExpr *chosenExpr = randElement(TheExprs);

    std::vector<std::string> elements;
    for (unsigned i = 0; i < chosenExpr->getNumInits(); ++i) {
      Expr *initExpr = chosenExpr->getInit(i);
      elements.push_back(getSourceText(initExpr));
    }

    std::reverse(elements.begin(), elements.end());
    std::string newElements = "{ " + llvm::join(elements, ", ") + " }";

    getRewriter().ReplaceText(chosenExpr->getSourceRange(), newElements);

    return true;
  }

private:
  std::vector<InitListExpr*> TheExprs;
};

static RegisterMutator<ReverseArrayElements> M("u3.ReverseArrayElements", "This mutator selects an array initialization in the code and reverses the order of its elements.");
