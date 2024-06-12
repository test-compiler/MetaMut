#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class DecrementForLoopInit : public Mutator,
                      public clang::RecursiveASTVisitor<DecrementForLoopInit> {
public:
  using Mutator::Mutator;

  bool VisitForStmt(clang::ForStmt *FS) {
    if (FS->getInit() && isa<DeclStmt>(FS->getInit())) {
        DeclStmt *DS = cast<DeclStmt>(FS->getInit());
        for (auto it = DS->decl_begin(); it != DS->decl_end(); ++it) {
            if (VarDecl *VD = dyn_cast<VarDecl>(*it)) {
                if (VD->getInit() && findIntegerLiteral(VD->getInit())) {
                    TheLoops.push_back(FS);
                }
            }
        }
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLoops.empty()) return false;

    ForStmt *FS = randElement(TheLoops);
    DeclStmt *DS = cast<DeclStmt>(FS->getInit());

    for (auto it = DS->decl_begin(); it != DS->decl_end(); ++it) {
        if (VarDecl *VD = dyn_cast<VarDecl>(*it)) {
            IntegerLiteral *IL = findIntegerLiteral(VD->getInit());
            uint64_t Val = IL->getValue().getLimitedValue();
            if (Val == 0) continue;  // Skip 0 to prevent negative values
            
            getRewriter().ReplaceText(IL->getSourceRange(), std::to_string(Val - 1));
            return true;
        }
    }
    
    return false;
  }

  IntegerLiteral *findIntegerLiteral(Expr *E) {
    if (auto *IL = dyn_cast<IntegerLiteral>(E)) {
      return IL;
    } else {
      for (auto *Child : E->children()) {
        if (auto *IL = findIntegerLiteral(cast<Expr>(Child))) {
          return IL;
        }
      }
    }
    return nullptr;
  }

private:
  std::vector<clang::ForStmt *> TheLoops;
};

static RegisterMutator<DecrementForLoopInit> M("u3.DecrementForLoopInit", "This mutator selects a 'for' loop and decrements the initialisation statement by one, effectively changing the starting point of the loop.");
