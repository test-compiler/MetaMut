// WrapFunctionInConditional.cpp
#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class WrapFunctionInConditional : public Mutator,
                      public clang::RecursiveASTVisitor<WrapFunctionInConditional> {
public:
  using Mutator::Mutator;

  bool VisitCallExpr(clang::CallExpr *CE) {
    TheFunctions.push_back(CE);
    return true;
  }

  bool mutate() override {
    // Traverse the AST
    TraverseDecl(getASTContext().getTranslationUnitDecl());

    if (TheFunctions.empty()) return false;
    
    // Randomly select a function call
    CallExpr *CE = randElement(TheFunctions);

    // Generate a conditional statement
    std::string condition = "if(randBool()) ";
    std::string statement = getSourceText(CE);

    // Wrap the function call within the conditional statement
    std::string newStatement = condition + "{\n" + statement + ";\n}\n";
    getRewriter().ReplaceText(CE->getSourceRange(), newStatement);

    return true;
  }

private:
  std::vector<clang::CallExpr *> TheFunctions;
};

static RegisterMutator<WrapFunctionInConditional> M("u3.WrapFunctionInConditional", "Wrap function call with a conditional statement");
