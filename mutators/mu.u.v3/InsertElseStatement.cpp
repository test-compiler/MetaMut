#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertElseStatement : public Mutator, public RecursiveASTVisitor<InsertElseStatement> {
public:
  using Mutator::Mutator;

  bool VisitIfStmt(IfStmt* stmt) {
    if (!stmt->getElse()) // Only if statements without else
      IfStmts.push_back(stmt);
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl* FD) {
    if (FD->getReturnType().getAsString() != "void") // Only non-void functions
      NonVoidFunctions.push_back(FD);
    return true;
  }

  bool mutate() override {
    IfStmts.clear();
    NonVoidFunctions.clear();
    TraverseAST(getASTContext());

    if (IfStmts.empty() || NonVoidFunctions.empty())
      return false; 

    IfStmt* stmt = randElement(IfStmts);
    FunctionDecl* FD = randElement(NonVoidFunctions); // Get a random non-void function
    
    std::string funcCall = FD->getNameAsString() + "();";
    std::string replacement = getSourceText(stmt) + " else { " + funcCall + " }";
    getRewriter().ReplaceText(stmt->getSourceRange(), replacement);

    return true;
  }

private:
  std::vector<IfStmt*> IfStmts;
  std::vector<FunctionDecl*> NonVoidFunctions;
};

static RegisterMutator<InsertElseStatement> M("u3.InsertElseStatement", "Insert 'else' branch with a non-void function call for an 'if' statement");
