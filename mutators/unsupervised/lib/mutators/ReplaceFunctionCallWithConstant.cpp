// ReplaceFunctionCallWithConstant.cpp
#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ReplaceFunctionCallWithConstant : public Mutator,
                                        public RecursiveASTVisitor<ReplaceFunctionCallWithConstant> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseTranslationUnitDecl(getASTContext().getTranslationUnitDecl());
    if (TheFunctionCalls.empty()) {
      return false;
    }

    CallExpr *functionCall = randElement(TheFunctionCalls);

    // Get the return type of the function
    QualType returnType = functionCall->getCallReturnType(getASTContext());

    // Generate a constant value based on the return type of the function
    std::string constantValue = generateConstantValue(returnType);

    // Replace the function call expression with the constant value
    SourceRange callRange = functionCall->getSourceRange();
    getRewriter().ReplaceText(callRange, constantValue);

    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    FunctionDecl *functionDecl = CE->getDirectCallee();
    if (functionDecl && !functionDecl->isInlined()) {  // Exclude inlined function calls
      TheFunctionCalls.push_back(CE);
    }
    return true;
  }

private:
  std::vector<CallExpr *> TheFunctionCalls;

  std::string generateConstantValue(QualType returnType) {
    if (returnType->isBooleanType()) {
      return "false";  // Replace boolean function calls with false
    } else if (returnType->isIntegerType()) {
      return "0";  // Replace integer function calls with 0
    } else if (returnType->isFloatingType()) {
      return "0.0";  // Replace floating-point function calls with 0.0
    } else if (returnType->isPointerType()) {
      return "nullptr";  // Replace pointer function calls with nullptr
    } else {
      return "42";  // Replace function calls with the constant value 42 as a fallback
    }
  }
};

static RegisterMutator<ReplaceFunctionCallWithConstant> M(
    "ReplaceFunctionCallWithConstant", "Replace a function call with a constant value.");