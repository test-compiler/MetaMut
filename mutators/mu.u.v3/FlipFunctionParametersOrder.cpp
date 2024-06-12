// FlipFunctionParametersOrder.cpp
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class FlipFunctionParametersOrder : public Mutator,
                                    public RecursiveASTVisitor<FlipFunctionParametersOrder> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (FD->getNumParams() < 2) return true; // Skip if less than 2 parameters
    TheFunctions.push_back(FD);
    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    if (FunctionDecl *FD = CE->getDirectCallee()) {
      TheCalls[FD].push_back(CE);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheFunctions.empty()) return false;

    FunctionDecl *FD = randElement(TheFunctions);

    // Reverse parameters in function declaration
    SourceRange range = findBracesRange(FD->getBeginLoc(), '(', ')');
    std::vector<std::string> params;
    for (unsigned i = 0; i < FD->getNumParams(); ++i) {
      params.push_back(getSourceText(FD->getParamDecl(i)));
    }
    std::reverse(params.begin(), params.end());
    getRewriter().ReplaceText(range, "(" + llvm::join(params, ", ") + ")");

    // Reverse arguments in all call expressions to the function
    for (auto *CE : TheCalls[FD]) {
      range = findBracesRange(CE->getBeginLoc(), '(', ')');
      std::vector<std::string> args;
      for (unsigned i = 0; i < CE->getNumArgs(); ++i) {
        args.push_back(getSourceText(CE->getArg(i)));
      }
      std::reverse(args.begin(), args.end());
      getRewriter().ReplaceText(range, "(" + llvm::join(args, ", ") + ")");
    }

    return true;
  }

private:
  std::vector<FunctionDecl *> TheFunctions;
  std::map<FunctionDecl *, std::vector<CallExpr *>> TheCalls;
};

static RegisterMutator<FlipFunctionParametersOrder> M("u3.FlipFunctionParametersOrder", 
    "Identifies a function definition and a function call in the code, then flips the order of parameters in both the function definition and all its function calls."
);
