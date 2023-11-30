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

class ScrambleFunctionArgs
    : public Mutator,
      public clang::RecursiveASTVisitor<ScrambleFunctionArgs> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheCalls.empty()) return false;

    CallExpr *call = randElement(TheCalls);
    std::vector<Expr *> args(call->arg_begin(), call->arg_end());
    std::map<QualType, std::vector<Expr *>> typedArgs;
    for (auto arg : args) {
      typedArgs[arg->getType()].push_back(arg);
    }
    for (auto &pair : typedArgs) {
      std::shuffle(pair.second.begin(), pair.second.end(), getManager().getRandomGenerator());
    }
    std::string newArgs;
    for (auto arg : args) {
      if (!newArgs.empty()) newArgs += ", ";
      newArgs += getSourceText(typedArgs[arg->getType()].back()).str();
      typedArgs[arg->getType()].pop_back();
    }
    SourceRange braces = findBracesRange(call->getCallee()->getEndLoc());
    getRewriter().ReplaceText(braces, "(" + newArgs + ")");

    return true;
  }

  bool VisitCallExpr(clang::CallExpr *CE) {
    TheCalls.push_back(CE);
    return true;
  }

private:
  std::vector<clang::CallExpr *> TheCalls;
};

static RegisterMutator<ScrambleFunctionArgs> M(
    "ScrambleFunctionArgs", "Randomly shuffle function arguments while maintaining type compatibility.");