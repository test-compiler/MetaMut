#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceVarWithNullPointer : public Mutator,
                                  public clang::RecursiveASTVisitor<ReplaceVarWithNullPointer> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    if (VD->getType()->isPointerType()) {
      PointerDecls.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (PointerDecls.empty()) return false;

    VarDecl *oldDecl = randElement(PointerDecls);

    // Separate the declarations from their initializations
    auto init = oldDecl->getInit();
    if (init) {
      // Check if the initializer is directly part of the declaration
      auto declText = Lexer::getSourceText(CharSourceRange::getTokenRange(oldDecl->getSourceRange()), 
      getASTContext().getSourceManager(), getASTContext().getLangOpts());
      if (declText.find('=') != std::string::npos) {
        // Replace the entire declaration with pointer type
        std::string newDecl = formatAsDecl(oldDecl->getType(), oldDecl->getNameAsString()) + ";\n" + oldDecl->getNameAsString() + " = " + getSourceText(init) + ";";
        getRewriter().ReplaceText(oldDecl->getSourceRange(), newDecl);
      } else {
        // Remove the initializer
        getRewriter().ReplaceText(init->getSourceRange(), "");
        getRewriter().InsertTextAfter(getLocForEndOfToken(oldDecl->getEndLoc()), "\n" + oldDecl->getNameAsString() + " = " + getSourceText(init) + ";");
      }
    }

    // Nullify the pointer immediately after its declaration
    std::string nullifyStmt = oldDecl->getNameAsString() + " = NULL;";
    getRewriter().InsertTextAfter(getLocForEndOfToken(oldDecl->getEndLoc()), "\n" + nullifyStmt);

    return true;
  }

private:
  std::vector<VarDecl*> PointerDecls;
};

static RegisterMutator<ReplaceVarWithNullPointer> M("u3.ReplaceVarWithNullPointer", "Replace a pointer variable with NULL");
