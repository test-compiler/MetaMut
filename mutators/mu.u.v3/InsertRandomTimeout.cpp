#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertRandomTimeout : public Mutator,
                            public clang::RecursiveASTVisitor<InsertRandomTimeout> {
public:
  using Mutator::Mutator;

  bool VisitCallExpr(CallExpr *CE) {
    TheCalls.push_back(CE);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheCalls.empty()) return false;

    CallExpr *selectedCall = randElement(TheCalls);

    // Simulating timeout mechanism with if-else
    unsigned int timeoutValue = getManager().getRandomGenerator()(); // get a random timeout value
    std::string callText = getSourceText(selectedCall);
    std::string timeoutCheck = "if (rand() % 100 < " + std::to_string(timeoutValue) + ") {\n    " + callText + ";\n}\nelse {\n    // Timed out\n}";

    // Replace the original function call with the new timeout mechanism
    getRewriter().ReplaceText(selectedCall->getSourceRange(), timeoutCheck);

    return true;
  }

private:
  std::vector<CallExpr *> TheCalls;
};

static RegisterMutator<InsertRandomTimeout> M("u3.InsertRandomTimeout", 
    "This mutator selects a random function call and wraps it with a simulated timeout mechanism using if-else statements and a random time limit.");
