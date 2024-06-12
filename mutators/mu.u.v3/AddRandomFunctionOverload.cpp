#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class AddRandomFunctionOverload : public Mutator, 
                                  public clang::RecursiveASTVisitor<AddRandomFunctionOverload> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFunctions.empty()) return false;

    FunctionDecl *functionToChange = randElement(TheFunctions);
    std::string oldFuncName = functionToChange->getNameInfo().getAsString();

    // Generating new parameter
    QualType newType = getRandomType();
    std::string newParamName = generateUniqueName("newParam");
    std::string newParamDecl = formatAsDecl(newType, newParamName);
    
    // Inserting new parameter inside the function declaration
    SourceRange bracesRange = findBracesRange(functionToChange->getBeginLoc());
    SourceLocation insertLocation = bracesRange.getBegin().getLocWithOffset(1); // offset is to get inside the parentheses
    getRewriter().InsertTextAfter(insertLocation, newParamDecl + ", ");

    // Replace calls to the function with the new overloaded version
    for (CallExpr* funcCall : TheFuncCalls) {
        FunctionDecl* calledFunc = funcCall->getDirectCallee();
        if (calledFunc->getNameInfo().getAsString() == oldFuncName) {
            SourceRange bracesRange = findBracesRange(funcCall->getBeginLoc());
            SourceLocation insertLocation = bracesRange.getBegin().getLocWithOffset(1); // offset is to get inside the parentheses
            std::string newArg = "0, ";
            getRewriter().InsertTextAfter(insertLocation, newArg);
        }
    }
    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (FD->getNameInfo().getAsString() != "main") {
      TheFunctions.push_back(FD);
    }
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *CE) {
    TheFuncCalls.push_back(CE);
    return true;
  }

private:
  QualType getRandomType() {
    std::vector<QualType> types = {
      getASTContext().IntTy, getASTContext().FloatTy,
      getASTContext().DoubleTy, getASTContext().CharTy
    };
    return types[randIndex(types.size())];
  }

  std::vector<FunctionDecl *> TheFunctions;
  std::vector<CallExpr *> TheFuncCalls;
};

static RegisterMutator<AddRandomFunctionOverload> M("u3.AddRandomFunctionOverload", 
    "Create an overloaded version of the function with an additional random type parameter and replace the original one with the new version.");
