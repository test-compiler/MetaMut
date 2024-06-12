#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ConvertBuiltinToCustomFunc : public Mutator,
                                   public clang::RecursiveASTVisitor<ConvertBuiltinToCustomFunc> {
public:
  using Mutator::Mutator;

  bool VisitCallExpr(CallExpr *CE) {
    if (FunctionDecl *FD = CE->getDirectCallee()) {
      if (FD->isExternC()) {
        TheCalls.push_back(CE);
      }
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheCalls.empty()) return false;

    CallExpr *CE = randElement(TheCalls);
    FunctionDecl *FD = CE->getDirectCallee();
    std::string funcName = FD->getNameInfo().getName().getAsString();
    
    std::string newFuncName = funcName + "_custom";
    SourceLocation loc = CE->getBeginLoc();
    getRewriter().ReplaceText(loc, funcName.size(), newFuncName);

    return true;
  }

private:
  std::vector<CallExpr *> TheCalls;
};

static RegisterMutator<ConvertBuiltinToCustomFunc> M("u3.ConvertBuiltinToCustomFunc", "Replaces calls to builtin C++ functions with user-defined equivalent");
