#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class InsertRandomTypeQualifier : public Mutator,
                                  public clang::RecursiveASTVisitor<InsertRandomTypeQualifier> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheFuncDecls.empty()) return false;

    // Randomly select a function declaration
    FunctionDecl *decl = randElement(TheFuncDecls);

    // Get the current return type
    QualType type = decl->getReturnType();

    // If type is void, there's nothing to mutate
    if (type->isVoidType()) return false;

    // Randomly decide whether to add, modify, or remove a qualifier
    int action = randIndex(3);

    if (action == 0 && !type.isConstQualified()) {
      // Add const qualifier
      type.addConst();
    } else if (action == 1 && !type.isVolatileQualified()) {
      // Add volatile qualifier
      type.addVolatile();
    } else if (action == 2 && type.isConstQualified()) {
      // Remove const qualifier
      type.removeLocalConst();
    } else {
      return false;
    }

    std::string newDecl = decl->getNameAsString() + "(";

    for (unsigned i = 0; i < decl->getNumParams(); ++i) {
      if (i != 0) newDecl += ", ";
      newDecl += formatAsDecl(decl->getParamDecl(i)->getType(), decl->getParamDecl(i)->getNameAsString());
    }

    newDecl += ")";

    if (decl->isThisDeclarationADefinition()) newDecl += " {}";

    getRewriter().ReplaceText(decl->getSourceRange(), newDecl);

    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (FD->isThisDeclarationADefinition() && !FD->getReturnType()->isVoidType()) TheFuncDecls.push_back(FD);
    return true;
  }

private:
  std::vector<clang::FunctionDecl *> TheFuncDecls;
};

static RegisterMutator<InsertRandomTypeQualifier> M(
    "InsertRandomTypeQualifier",
    "Randomly add, modify, or remove a type qualifier (const, volatile) to the return type of a FunctionDecl.");