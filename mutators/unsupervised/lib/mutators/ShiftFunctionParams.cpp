#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ShiftFunctionParams : public Mutator,
                            public RecursiveASTVisitor<ShiftFunctionParams> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    TheFunctions.push_back(FD);
    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    if (FunctionDecl *calledFunction = CE->getDirectCallee()) {
      TheCallSites[calledFunction].push_back(CE);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheFunctions.empty())
      return false;

    FunctionDecl *function = randElement(TheFunctions);
    auto params = function->parameters();

    if (params.size() < 2)
      return false;

    std::rotate(params.begin(), params.begin() + 1, params.end());

    for (ParmVarDecl *param : params) {
      getRewriter().ReplaceText(param->getSourceRange(), param->getNameAsString());
    }

    for (auto &callSite : TheCallSites[function]) {
      std::vector<std::string> args;

      for (unsigned i = 0; i < callSite->getNumArgs(); ++i) {
        args.push_back(getSourceText(callSite->getArg(i)).str());
      }

      std::rotate(args.begin(), args.begin() + 1, args.end());

      for (unsigned i = 0; i < callSite->getNumArgs(); ++i) {
        getRewriter().ReplaceText(callSite->getArg(i)->getSourceRange(), args[i]);
      }
    }

    return true;
  }

private:
  std::vector<FunctionDecl *> TheFunctions;
  std::map<FunctionDecl *, std::vector<CallExpr *>> TheCallSites;

};

static RegisterMutator<ShiftFunctionParams> X("ShiftFunctionParams", "Shift the position of all parameters one place to the right.");