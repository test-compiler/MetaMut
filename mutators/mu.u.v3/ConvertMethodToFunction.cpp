#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ConvertMethodToFunction : public Mutator,
                      public clang::RecursiveASTVisitor<ConvertMethodToFunction> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    TheFunctions.push_back(FD);
    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    TheCalls.push_back(CE);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFunctions.empty()) return false;

    FunctionDecl *function = randElement(TheFunctions);

    // Check if the mutation can be applied
    if (function->isInvalidDecl()) return false;

    // Step 1: Create the standalone function declaration
    std::string returnType = function->getReturnType().getAsString();
    std::string functionName = function->getNameAsString();
    std::string argListStr = function->getParamDecl(0)->getNameAsString();

    for (int i = 1; i < function->getNumParams(); ++i) {
      argListStr += ", " + function->getParamDecl(i)->getNameAsString();
    }
    
    std::string newFunctionName = generateUniqueName("new_func");
    std::string newFunction = returnType + " " + newFunctionName + "(" + argListStr + ") {\n";
    newFunction += "    return " + functionName + "(" + argListStr + ");\n";
    newFunction += "}\n";
    
    // Insert new function before original
    addStringBeforeFunctionDecl(function, newFunction);

    // Replace references to original function with new function
    for (auto CE : TheCalls) {
      if (auto calledFunc = CE->getDirectCallee()) {
        if (calledFunc == function) {
          // Replace the function call with the new function
          getRewriter().ReplaceText(CE->getCallee()->getSourceRange(), newFunctionName);
        }
      }
    }

    // If code reaches this point, a mutation has been made
    return true;
  }

private:
  std::vector<FunctionDecl*> TheFunctions;
  std::vector<CallExpr*> TheCalls;
};

static RegisterMutator<ConvertMethodToFunction> M("u3.ConvertMethodToFunction", 
    "Selects a function and creates a new function that calls the original function, replacing all uses of the original function with the new function");
