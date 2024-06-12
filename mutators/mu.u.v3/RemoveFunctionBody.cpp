#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class RemoveFunctionBody
    : public Mutator,
      public clang::RecursiveASTVisitor<RemoveFunctionBody> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (!FD->getReturnType()->isVoidType()) {
      TheFunctions.push_back(FD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if(TheFunctions.empty())
      return false;

    FunctionDecl *funcDecl = randElement(TheFunctions);
    QualType retType = funcDecl->getReturnType();
    std::string newValue = getDefaultInitValue(retType);

    // remove the old function body
    getRewriter().RemoveText(funcDecl->getBody()->getSourceRange());

    // insert the new function body
    std::string newBody = " { return " + newValue + "; }";
    getRewriter().InsertTextAfter(getLocForEndOfToken(funcDecl->getEndLoc()), newBody);

    return true;
  }

  // Returns the default value for a given type
  std::string getDefaultInitValue(QualType QT) {
    if (QT->isBooleanType()) { return "false"; }
    else if (QT->isIntegerType()) { return "0"; }
    else if (QT->isRealFloatingType()) { return "0.0"; }
    else if (QT->isPointerType()) { return "NULL"; }
    else if (QT->isRecordType()) { return "{}"; }
    else { return ""; }
  }

private:
  std::vector<FunctionDecl*> TheFunctions;
};

static RegisterMutator<RemoveFunctionBody> M("u3.RemoveFunctionBody", "Removes a function body, replacing it with a return statement returning a default value of the function's return type");
