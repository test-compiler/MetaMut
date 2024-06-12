#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertRedundantIfStmt : public Mutator, public RecursiveASTVisitor<InsertRedundantIfStmt> {
public:
  using Mutator::Mutator;
  
  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (CompoundStmt *CS = dyn_cast_or_null<CompoundStmt>(FD->getBody())) {
      for (auto *S : CS->body()) {
        TheStmts.push_back(S);
      }
    }
    return true;
  }
  
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheStmts.empty()) return false;
    
    Stmt *selectedStmt = randElement(TheStmts);
    
    std::string sourceStmt = getSourceText(selectedStmt);

    std::string newStmt = "if (1) {\n" + sourceStmt + "\n}";

    getRewriter().ReplaceText(selectedStmt->getSourceRange(), newStmt);

    return true;
  }
  
private:
  std::vector<Stmt *> TheStmts;
};

static RegisterMutator<InsertRedundantIfStmt> M("u3.InsertRedundantIfStmt",
    "Insert redundant if statement with always true condition.");
