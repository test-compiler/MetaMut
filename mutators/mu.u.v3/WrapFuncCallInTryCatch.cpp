#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <stdio.h>

using namespace clang;

class WrapFuncCallInTryCatch : public Mutator,
                               public clang::RecursiveASTVisitor<WrapFuncCallInTryCatch> {
public:
  using Mutator::Mutator;

  bool VisitCallExpr(CallExpr *CE) {
    TheFuncCalls.push_back(CE);
    return true;
  }

  bool mutate() override {
    // Traverse the AST
    TraverseAST(getASTContext());

    // No function calls in the code
    if (TheFuncCalls.empty()) return false;

    // Randomly select a function call
    CallExpr *selectedFuncCall = randElement(TheFuncCalls);

    // Generate unique name for the error status variable
    std::string errVarName = generateUniqueName("err");

    // Construct the error-checking block
    std::string errorCheckingBlock = 
        "int " + errVarName + " = " + getSourceText(selectedFuncCall) + ";\n"
        "if (" + errVarName + " != 0) {\n"
        "  fprintf(stderr, \"Error occurred in function call, error code: %d\", " + errVarName + ");\n"
        "}\n";

    // Replace the function call with the error-checking block
    getRewriter().ReplaceText(selectedFuncCall->getSourceRange(), errorCheckingBlock);

    return true;
  }

private:
  std::vector<CallExpr *> TheFuncCalls;
};

static RegisterMutator<WrapFuncCallInTryCatch> M("u3.WrapFuncCallInTryCatch", "Wraps a function call and checks for error afterwards");
