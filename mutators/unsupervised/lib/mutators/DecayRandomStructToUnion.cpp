// DecayRandomStructToUnion.cpp
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class DecayRandomStructToUnion : public Mutator,
                                 public clang::RecursiveASTVisitor<DecayRandomStructToUnion> {

  using VisitorTy = clang::RecursiveASTVisitor<DecayRandomStructToUnion>;

public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheStructs.empty()) return false;

    RecordDecl *selectedStruct = randElement(TheStructs);

    SourceLocation structLoc = findStringLocationFrom(selectedStruct->getBeginLoc(), "struct");
    getRewriter().ReplaceText(structLoc, 6, "union");

    for (auto usage : TheVarDecls) {
      if (usage->getType()->getAsStructureType()->getDecl() == selectedStruct) {
        auto init = usage->getInit();
        if (init && isa<InitListExpr>(init)) {
          auto list = cast<InitListExpr>(init);
          if (list->getNumInits() > 0) {
            getRewriter().ReplaceText(init->getSourceRange(), getSourceText(list->getInit(0)).str());
          }
        }
      }
    }
    return true;
  }

  bool VisitRecordDecl(clang::RecordDecl *RD) {
    if (RD->isStruct()) {
      TheStructs.push_back(RD);
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getType()->isStructureType()) {
      TheVarDecls.push_back(VD);
    }
    return true;
  }

private:
  std::vector<clang::RecordDecl *> TheStructs;
  std::vector<clang::VarDecl *> TheVarDecls;
};

static RegisterMutator<DecayRandomStructToUnion> X(
    "DecayRandomStructToUnion", "Randomly select a struct declaration and convert it into a union");