#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ModifyFunctionDefaultArgument : public Mutator,
                                      public clang::RecursiveASTVisitor<ModifyFunctionDefaultArgument> {
public:
  using Mutator::Mutator;
  
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFunctions.empty()) return false;

    FunctionDecl *func = randElement(TheFunctions);

    auto params = func->parameters();
    if (params.empty()) return false;

    ParmVarDecl *param = nullptr;
    for (auto *p : params) {
      if (p->hasDefaultArg()) {
        param = p;
        break;
      }
    }
    
    if (!param) return false;
    
    auto defArg = param->getDefaultArg();
    std::string newDefaultArg = generateUniqueName("newArg");
    getRewriter().ReplaceText(defArg->getSourceRange(), newDefaultArg);

    for (auto *call : TheCallExprs) {
      if (call->getDirectCallee() == func) {
        unsigned argCount = std::distance(call->arg_begin(), call->arg_end());
        if (argCount < params.size()) {
          getRewriter().InsertTextAfter(getLocForEndOfToken(call->getEndLoc()), ", " + newDefaultArg);
        }
      }
    }

    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (FD->doesThisDeclarationHaveABody() && FD->isThisDeclarationADefinition()) {
      TheFunctions.push_back(FD);
    }
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *CE) {
    TheCallExprs.push_back(CE);
    return true;
  }

private:
  std::vector<clang::FunctionDecl *> TheFunctions;
  std::vector<clang::CallExpr *> TheCallExprs;
};

static RegisterMutator<ModifyFunctionDefaultArgument> M(
    "ModifyFunctionDefaultArgument", "Modify function's default argument and adjust all call sites accordingly.");