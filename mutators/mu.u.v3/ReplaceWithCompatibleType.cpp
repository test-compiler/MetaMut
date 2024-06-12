#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class ReplaceWithCompatibleType : public Mutator,
                                  public clang::RecursiveASTVisitor<ReplaceWithCompatibleType> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    if (!isa<FieldDecl>(VD)) {
      TheVars.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVars.empty()) return false;

    VarDecl *chosenVar = randElement(TheVars);
    QualType oldType = chosenVar->getType();

    // Prepare a new compatible type (here, for simplicity, we replace int with long)
    if (oldType.getAsString() == "int") {
        QualType newType = getASTContext().LongTy; // Use predefined QualType for long
        std::string replacedVar = formatAsDecl(newType, chosenVar->getNameAsString()); // formatAsDecl generates the proper declaration string
        getRewriter().ReplaceText(chosenVar->getSourceRange(), replacedVar);
    } else {
        return false;
    }

    return true;
  }

private:
  std::vector<VarDecl*> TheVars;
};

static RegisterMutator<ReplaceWithCompatibleType> M("u3.ReplaceWithCompatibleType", "This mutator selects a variable declaration and changes its type to a compatible type, preserving the semantics of the program.");
