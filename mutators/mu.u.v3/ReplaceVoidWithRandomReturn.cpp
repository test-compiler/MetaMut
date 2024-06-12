#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceVoidWithRandomReturn : public Mutator,
                                    public clang::RecursiveASTVisitor<ReplaceVoidWithRandomReturn> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if(FD->getReturnType()->isVoidType() && FD->hasBody()) {
        TheFunctions.push_back(FD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if(TheFunctions.empty()) return false;

    FunctionDecl *funcDecl = randElement(TheFunctions);

    auto& Context = getASTContext();

    // Choose a return type randomly
    QualType newType = Context.IntTy;
    if(randBool()) {
        newType = Context.CharTy;
    }

    // Include any qualifiers of the return type
    std::string newFuncType;
    newType.getAsStringInternal(newFuncType, Context.getPrintingPolicy());

    // Get the original return type
    auto returnType = funcDecl->getReturnType();
    std::string origReturnType;
    returnType.getAsStringInternal(origReturnType, Context.getPrintingPolicy());

    // Create the new declaration
    std::string newFuncDecl = getSourceText(funcDecl);
    size_t pos = newFuncDecl.find(origReturnType);
    if (pos != std::string::npos) {
        // Replace the original return type with the new one
        newFuncDecl.replace(pos, origReturnType.length(), newFuncType);
    }

    // Replace the declaration with the new one
    getRewriter().ReplaceText(funcDecl->getSourceRange(), newFuncDecl);

    // Modify the body to add a return statement
    CompoundStmt *CS = cast<CompoundStmt>(funcDecl->getBody());

    // Check for a suitable return value
    std::string returnValue = "0";
    if(newType == Context.CharTy) {
        returnValue = "'a'";
    }

    // Append a return statement at the end of the function
    std::string newStmt = "return " + returnValue + ";";
    getRewriter().InsertText(CS->getRBracLoc(), newStmt);

    return true;
  }

private:
  std::vector<clang::FunctionDecl *> TheFunctions;
};

static RegisterMutator<ReplaceVoidWithRandomReturn> M("u3.ReplaceVoidWithRandomReturn", "Change a void function to return a random type and add a return statement");
