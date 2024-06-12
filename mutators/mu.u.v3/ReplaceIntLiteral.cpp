#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceIntLiteral : public Mutator,
                      public clang::RecursiveASTVisitor<ReplaceIntLiteral> {
public:
  using Mutator::Mutator;

  bool VisitIntegerLiteral(IntegerLiteral *IL) {
    TheLiterals.push_back(IL);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLiterals.empty()) return false;

    IntegerLiteral *IL = randElement(TheLiterals);
    
    int randomInt = rand(); // You can replace this with your own random number generator.
    std::string randomIntStr = std::to_string(randomInt);

    getRewriter().ReplaceText(IL->getSourceRange(), randomIntStr);

    return true;
  }

private:
  std::vector<IntegerLiteral*> TheLiterals;
};

static RegisterMutator<ReplaceIntLiteral> M("u3.ReplaceIntLiteral", "This mutator selects an integer literal in the code and replaces it with another random integer literal.");
