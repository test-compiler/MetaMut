// InterchangeFunctionCalls.cpp
#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class InterchangeFunctionCalls : public Mutator,
                                 public RecursiveASTVisitor<InterchangeFunctionCalls> {
public:
  using Mutator::Mutator;

  bool VisitCallExpr(clang::CallExpr *CE) {
    TheCallExpressions.push_back(CE);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheCallExpressions.size() < 2)
      return false;

    CallExpr *selectedCE1 = randElement(TheCallExpressions);
    CallExpr *selectedCE2 = randElement(TheCallExpressions);

    if (selectedCE1 == selectedCE2) 
      return false;

    std::string sourceText1 = getSourceText(selectedCE1);
    std::string sourceText2 = getSourceText(selectedCE2);

    getRewriter().ReplaceText(selectedCE1->getSourceRange(), sourceText2);
    getRewriter().ReplaceText(selectedCE2->getSourceRange(), sourceText1);

    return true;
  }

private:
  std::vector<clang::CallExpr *> TheCallExpressions;

  std::string getSourceText(const clang::CallExpr *CE) {
    return Lexer::getSourceText(
        CharSourceRange::getTokenRange(CE->getSourceRange()), 
        getASTContext().getSourceManager(), 
        getCompilerInstance().getLangOpts()).str();
  }
};

static RegisterMutator<InterchangeFunctionCalls> M("u3.InterchangeFunctionCalls", 
    "Identifies two consecutive function calls in the code and swaps their order of execution");
