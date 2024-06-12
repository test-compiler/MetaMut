#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceFuncParameterWithDefault : public Mutator,
                      public clang::RecursiveASTVisitor<ReplaceFuncParameterWithDefault> {

public:
  using Mutator::Mutator;
  
  bool VisitCallExpr(clang::CallExpr *CE) {
    for (auto arg : CE->arguments()) {
      // Collect call expressions along with their arguments
      TheCallsAndArgs[CE].push_back(arg);
    }
    return true;
  }

  bool mutate() override {
    // Step 1: Traverse the AST
    TraverseAST(getASTContext());
    
    // Step 2: Randomly select a mutation instance
    if (TheCallsAndArgs.empty()) return false;
    auto it = std::next(TheCallsAndArgs.begin(), randIndex(TheCallsAndArgs.size()));
    auto selectedCallExpr = it->first;
    auto &args = it->second;

    if (args.empty()) return false;
    auto selectedArg = randElement(args);

    // Step 3: Replace the argument with the default value of its type
    std::string defaultValue;
    clang::QualType qt = selectedArg->getType();
    if (qt->isIntegerType()) {
      defaultValue = "0";
    } else if (qt->isPointerType()) {
      defaultValue = "NULL";
    } else if (qt->isRealFloatingType()) {
      defaultValue = "0.0";
    } else if (qt->isBooleanType()) {
      defaultValue = "false";
    } else {
      // For other non-primitive types, we don't mutate
      return false;
    }
    
    getRewriter().ReplaceText(selectedArg->getSourceRange(), defaultValue);
    
    return true;
  }

private:
  std::map<clang::CallExpr *, std::vector<clang::Expr *>> TheCallsAndArgs;
};

static RegisterMutator<ReplaceFuncParameterWithDefault> M("u3.ReplaceFuncParameterWithDefault", 
    "Replace a function call argument with a default value of its type");
