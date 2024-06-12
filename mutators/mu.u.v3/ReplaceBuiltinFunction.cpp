#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceBuiltinFunction : public Mutator,
                               public clang::RecursiveASTVisitor<ReplaceBuiltinFunction> {
public:
  using Mutator::Mutator;

  bool VisitCallExpr(CallExpr *CE) {
    if (FunctionDecl *funcDecl = CE->getDirectCallee()) {
      TheFunctionCalls.push_back(CE);
    }
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->isThisDeclarationADefinition()) return true; // skip function declarations without a body
    TheFunctionDecls.push_back(FD);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFunctionCalls.empty()) return false;

    CallExpr *chosenFunctionCall = randElement(TheFunctionCalls);
    FunctionDecl *oldFunctionDecl = chosenFunctionCall->getDirectCallee();

    // Filter out potential replacement functions with matching parameters count and type
    std::vector<FunctionDecl*> possibleReplacementFunctions;
    for (auto functionDecl : TheFunctionDecls) {
      if (functionDecl->getDeclName() != oldFunctionDecl->getDeclName() &&
          functionDecl->getNumParams() == oldFunctionDecl->getNumParams() &&
          std::equal(functionDecl->param_begin(), functionDecl->param_end(), oldFunctionDecl->param_begin(), 
          [](const ParmVarDecl *a, const ParmVarDecl *b) {
            return a->getType() == b->getType();
          })) {
        possibleReplacementFunctions.push_back(functionDecl);
      }
    }

    std::string replacementFunctionName;

    // If no suitable function replacements are found, generate a new function
    if (possibleReplacementFunctions.empty()) {
        std::stringstream newFunction;
        replacementFunctionName = generateUniqueName("generated_func");
        newFunction << "void " << replacementFunctionName << "(";
        for (unsigned int i = 0; i < oldFunctionDecl->getNumParams(); i++) {
            if (i > 0)
                newFunction << ", ";
            newFunction << formatAsDecl(oldFunctionDecl->getParamDecl(i)->getType(), "p" + std::to_string(i));
        }
        newFunction << ") {}";
        newFunction << "\n"; // Add a newline to the end of the new function declaration
        
        // Assuming main function is the last function in the file
        FunctionDecl* mainFunctionDecl = TheFunctionDecls.back();
        if(mainFunctionDecl && mainFunctionDecl->getIdentifier()->getNameStart() == "main"){
            addStringBeforeFunctionDecl(mainFunctionDecl, newFunction.str());
        }
    } else {
        FunctionDecl *newFunctionDecl = randElement(possibleReplacementFunctions);
        replacementFunctionName = newFunctionDecl->getNameAsString();
    }

    // Check if the chosenFunctionCall or its callee is null before calling ReplaceText
    if (chosenFunctionCall == nullptr || chosenFunctionCall->getCallee() == nullptr) {
        return false;
    }

    auto calleeRange = chosenFunctionCall->getCallee()->getSourceRange();
    if (getASTContext().getSourceManager().isWrittenInMainFile(calleeRange.getBegin())
        && getASTContext().getSourceManager().isWrittenInMainFile(calleeRange.getEnd())) {
      getRewriter().ReplaceText(calleeRange, replacementFunctionName);
    } else {
      return false;
    }

    return true;
  }

private:
  std::vector<CallExpr*> TheFunctionCalls;
  std::vector<FunctionDecl*> TheFunctionDecls;
};

static RegisterMutator<ReplaceBuiltinFunction> M("u3.ReplaceBuiltinFunction", "Replace a function call with a different function call with the same number and type of parameters.");
