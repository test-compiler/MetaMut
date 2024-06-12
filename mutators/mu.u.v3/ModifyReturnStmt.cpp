#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ModifyReturnStmt : public Mutator,
                         public RecursiveASTVisitor<ModifyReturnStmt> {
public:
  using Mutator::Mutator;

  bool VisitReturnStmt(ReturnStmt* RS) {
    Returns.push_back(RS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (Returns.empty()) return false;

    ReturnStmt* selectedReturn = randElement(Returns);

    std::string returnText = getSourceText(selectedReturn);
    std::string modifiedReturn = "return " + returnText.substr(7) + " + 1;"; // Add 1 to the return value

    getRewriter().ReplaceText(selectedReturn->getSourceRange(), modifiedReturn);

    return true;
  }

private:
  std::vector<ReturnStmt*> Returns;  // Store the return statements
};

static RegisterMutator<ModifyReturnStmt> M("u3.ModifyReturnStmt", "This mutator selects a return statement in a function and modifies the value that is being returned");
