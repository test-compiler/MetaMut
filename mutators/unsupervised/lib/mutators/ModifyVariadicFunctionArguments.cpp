#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ModifyVariadicFunctionArguments : public Mutator,
                                        public clang::RecursiveASTVisitor<ModifyVariadicFunctionArguments> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheCallExprs.empty()) return false;

    CallExpr *call = randElement(TheCallExprs);

    // Create a new argument
    std::string newArg = "\"Mutated\"";

    // Add the new argument to the existing ones
    SourceRange argsRange(call->getArg(0)->getBeginLoc(), call->getArg(call->getNumArgs() - 1)->getEndLoc());
    std::string args = Lexer::getSourceText(CharSourceRange::getTokenRange(argsRange), getASTContext().getSourceManager(), getASTContext().getLangOpts()).str();
    getRewriter().ReplaceText(call->getSourceRange(), call->getDirectCallee()->getNameInfo().getAsString() + "(" + args + ", " + newArg + ")");

    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    if (CE->getDirectCallee() && CE->getDirectCallee()->isVariadic()) {
      TheCallExprs.push_back(CE);
    }
    return true;
  }

private:
  std::vector<CallExpr *> TheCallExprs;
};

static RegisterMutator<ModifyVariadicFunctionArguments> X(
    "ModifyVariadicFunctionArguments", "Adds an argument to a variadic function call.");