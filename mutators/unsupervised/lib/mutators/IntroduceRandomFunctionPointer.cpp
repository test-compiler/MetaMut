#include <clang/AST/ASTContext.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class IntroduceRandomFunctionPointer : public Mutator, public RecursiveASTVisitor<IntroduceRandomFunctionPointer> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (FD->hasBody() && FD->getNameAsString() != "main") TheDecls.push_back(FD);
    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    if (CE->getDirectCallee() && CE->getDirectCallee()->getNameAsString() != "main") TheCalls.push_back(CE);
    return true;
  }

  bool mutate() override {
    TraverseDecl(getASTContext().getTranslationUnitDecl());

    if (TheDecls.empty() || TheCalls.empty()) return false;

    FunctionDecl *selectedDecl = randElement(TheDecls);
    CallExpr *selectedCall = randElement(TheCalls);

    // Generate function pointer declaration
    std::string funcPtrName = generateUniqueName("funcPtr");
    std::string funcPtrDecl = selectedDecl->getReturnType().getAsString() + " (*" + funcPtrName + ")(";

    for (unsigned i = 0; i < selectedDecl->getNumParams(); ++i) {
      if (i != 0) funcPtrDecl += ", ";
      funcPtrDecl += selectedDecl->getParamDecl(i)->getType().getAsString();
    }
    funcPtrDecl += ") = &" + selectedDecl->getNameAsString() + ";";

    // Generate function call
    std::string newCall = funcPtrName;
    if(selectedCall->getNumArgs() > 0) {
      newCall += "(";
      for (unsigned i = 0; i < selectedCall->getNumArgs(); ++i) {
        if (i != 0) newCall += ", ";
        newCall += Lexer::getSourceText(CharSourceRange::getTokenRange(selectedCall->getArg(i)->getSourceRange()), getRewriter().getSourceMgr(), LangOptions()).str();
      }
      newCall += ")";
    }

    newCall += ";";

    // Replace the function call
    getRewriter().ReplaceText(selectedCall->getSourceRange(), newCall);

    // Insert the function pointer declaration
    getRewriter().InsertTextAfter(getLocForEndOfToken(selectedDecl->getEndLoc()), "\n" + funcPtrDecl);

    return true;
  }

private:
  std::vector<FunctionDecl*> TheDecls;
  std::vector<CallExpr*> TheCalls;
};

static RegisterMutator<IntroduceRandomFunctionPointer> M("IntroduceRandomFunctionPointer", "Replace a random function call with a call to the function via a function pointer.");