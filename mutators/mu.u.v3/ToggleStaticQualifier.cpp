#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ToggleStaticQualifier : public Mutator,
                              public clang::RecursiveASTVisitor<ToggleStaticQualifier> {
public:
  using Mutator::Mutator;
  
  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->isStaticLocal()) {
      staticLocalVars.push_back(VD);
    } else if (VD->getStorageClass() == SC_None && VD->isLocalVarDecl()) {
      nonStaticLocalVars.push_back(VD);
    }
    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (FD->isStatic()) {
      staticFuncs.push_back(FD);
    } else if (FD->getStorageClass() == SC_None) {
      nonStaticFuncs.push_back(FD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (randBool()) {
      // Process variable declarations
      if (staticLocalVars.empty() && nonStaticLocalVars.empty()) return false;
      
      if (randBool() && !staticLocalVars.empty()) {
        // Remove 'static' from a static local variable
        VarDecl *VD = randElement(staticLocalVars);
        getRewriter().ReplaceText(VD->getInnerLocStart(), 6, ""); // 6 is length of "static"
      } else if (!nonStaticLocalVars.empty()) {
        // Add 'static' to a non-static local variable
        VarDecl *VD = randElement(nonStaticLocalVars);
        getRewriter().InsertTextBefore(VD->getBeginLoc(), "static ");
      }
    } else {
      // Process function declarations
      if (staticFuncs.empty() && nonStaticFuncs.empty()) return false;

      if (randBool() && !staticFuncs.empty()) {
        // Remove 'static' from a static function
        FunctionDecl *FD = randElement(staticFuncs);
        getRewriter().ReplaceText(FD->getInnerLocStart(), 6, ""); // 6 is length of "static"
      } else if (!nonStaticFuncs.empty()) {
        // Add 'static' to a non-static function
        FunctionDecl *FD = randElement(nonStaticFuncs);
        getRewriter().InsertTextBefore(FD->getBeginLoc(), "static ");
      }
    }

    return true;
  }

private:
  std::vector<clang::VarDecl *> staticLocalVars;
  std::vector<clang::VarDecl *> nonStaticLocalVars;
  std::vector<clang::FunctionDecl *> staticFuncs;
  std::vector<clang::FunctionDecl *> nonStaticFuncs;
};

static RegisterMutator<ToggleStaticQualifier> M("u3.ToggleStaticQualifier", "Toggle static qualifier of a function or variable declaration.");
