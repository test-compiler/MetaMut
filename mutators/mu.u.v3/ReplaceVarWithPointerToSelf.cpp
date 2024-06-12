#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceVarWithPointerToSelf : public Mutator,
                                    public clang::RecursiveASTVisitor<ReplaceVarWithPointerToSelf> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    if (!VD->hasLocalStorage()) {
      return true;
    }
    TheVars.push_back(VD);
    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr* expr) {
    if (expr->getDecl() == TheVar) {
      if(TheVar->getType()->isPointerType() && TheVar->getType()->getPointeeType()->isFunctionType()) {
        getRewriter().ReplaceText(expr->getSourceRange(), newName);
      } else if(TheVar->getType()->isRecordType()) {
        getRewriter().ReplaceText(expr->getSourceRange(), newName + "->");
      } else {
        getRewriter().ReplaceText(expr->getSourceRange(), "*" + newName);
      }
    }
    return true;
  }

  bool mutate() override {
    // Traverse the AST
    TraverseAST(getASTContext());

    // Check if there are any variables to replace
    if (TheVars.empty()) 
      return false;

    // Randomly select a variable to replace
    TheVar = randElement(TheVars);

    // Create new VarDecl with pointer type
    QualType QT = TheVar->getType();
    QT = getASTContext().getPointerType(QT);
    
    // Generate a unique name for the new variable
    newName = generateUniqueName(TheVar->getNameAsString() + "_ptr");

    // Generate a new declaration for the pointer variable
    std::string newDeclStr = ";\n" + formatAsDecl(QT, newName) + " = &" + TheVar->getNameAsString() + ";\n";

    // Insert the new pointer variable declaration after the original variable declaration
    getRewriter().InsertTextAfter(getLocForEndOfToken(TheVar->getEndLoc()), newDeclStr);

    // Visit each DeclRefExpr and replace the old variable with the dereferenced new pointer variable
    TraverseAST(getASTContext());

    return true;
  }

private:
  std::vector<VarDecl *> TheVars;
  VarDecl *TheVar;
  std::string newName;
};

static RegisterMutator<ReplaceVarWithPointerToSelf> M("u3.ReplaceVarWithPointerToSelf", 
    "Replace a variable with a pointer to itself in local scopes.");
