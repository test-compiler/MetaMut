#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/Builtins.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ReplaceRandomBuiltinFunction
    : public Mutator,
      public clang::RecursiveASTVisitor<ReplaceRandomBuiltinFunction> {
public:
  using Mutator::Mutator;
  
  bool mutate() override {
    TraverseAST(getASTContext());
    if (BuiltinCalls.empty()) return false;

    CallExpr *call = randElement(BuiltinCalls);

    FunctionDecl *builtinDecl = call->getDirectCallee();
    if (builtinDecl == nullptr) return false;

    auto alternatives = computeValidAlternatives(builtinDecl);
    if (alternatives.empty()) return false;

    FunctionDecl *newFunc = randElement(alternatives);

    SourceRange callRange = call->getSourceRange();
    std::string replacement = newFunc->getNameAsString() + "(";
    for (unsigned i = 0; i < call->getNumArgs(); ++i) {
      if (i != 0) replacement += ", ";
      replacement += getSourceText(call->getArg(i)).str();
    }
    replacement += ")";
    getRewriter().ReplaceText(callRange, replacement);

    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    if (CE->getDirectCallee() && CE->getDirectCallee()->getBuiltinID() != 0)
      BuiltinCalls.push_back(CE);
    return true;
  }

private:
  std::vector<CallExpr *> BuiltinCalls;

  std::vector<FunctionDecl *> computeValidAlternatives(FunctionDecl *BuiltinDecl) {
    std::vector<FunctionDecl *> validAlternatives;

    auto &Context = getASTContext();
    TranslationUnitDecl *TU = Context.getTranslationUnitDecl();

    for (auto *D : TU->decls()) {
      if (FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
        if (FD->getBuiltinID() != 0 && FD->getNameAsString() != BuiltinDecl->getNameAsString() 
            && FD->getReturnType() == BuiltinDecl->getReturnType()) {
          validAlternatives.push_back(FD);
        }
      }
    }

    return validAlternatives;
  }
};

static RegisterMutator<ReplaceRandomBuiltinFunction> X(
    "ReplaceRandomBuiltinFunction", "Replace a random built-in function call with another function of the same return type.");