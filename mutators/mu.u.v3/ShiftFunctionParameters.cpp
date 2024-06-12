#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ShiftFunctionParameters : public Mutator,
                                public clang::RecursiveASTVisitor<ShiftFunctionParameters> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    // Only consider function with at least 2 parameters
    if (FD->param_size() > 1)
      TheFunctions.push_back(FD);
    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    if (FunctionDecl *FD = CE->getDirectCallee())
      TheCalls[FD].push_back(CE);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFunctions.empty()) return false;

    FunctionDecl *FD = randElement(TheFunctions);
    
    // Shift parameters in function declaration
    std::string lastParamStr = getSourceText(FD->getParamDecl(FD->getNumParams()-1));
    for (int i = FD->getNumParams() - 1; i > 0; --i) {
      std::string prevParamStr = getSourceText(FD->getParamDecl(i-1));
      getRewriter().ReplaceText(FD->getParamDecl(i)->getSourceRange(), prevParamStr);
    }
    getRewriter().ReplaceText(FD->getParamDecl(0)->getSourceRange(), lastParamStr);

    // Shift arguments in all function calls
    for (auto *CE : TheCalls[FD]) {
      std::string lastArgStr = getSourceText(CE->getArg(CE->getNumArgs()-1));
      for (int i = CE->getNumArgs() - 1; i > 0; --i) {
        std::string prevArgStr = getSourceText(CE->getArg(i-1));
        getRewriter().ReplaceText(CE->getArg(i)->getSourceRange(), prevArgStr);
      }
      getRewriter().ReplaceText(CE->getArg(0)->getSourceRange(), lastArgStr);
    }

    return true;
  }

private:
  std::vector<FunctionDecl *> TheFunctions;
  std::map<FunctionDecl *, std::vector<CallExpr *>> TheCalls;
};

static RegisterMutator<ShiftFunctionParameters> M("u3.ShiftFunctionParameters",
    "Identifies a function definition and a function call in the code, then cyclically shifts the order of parameters in both the function definition and all its function calls.");
