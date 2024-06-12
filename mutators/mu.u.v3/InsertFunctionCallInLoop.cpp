#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;

class InsertFunctionCallInLoop : public Mutator,
                                 public clang::RecursiveASTVisitor<InsertFunctionCallInLoop> {
public:
  using Mutator::Mutator;
  
  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->doesThisDeclarationHaveABody())
      return true;

    Functions.push_back(FD);
    return true;
  }

  bool VisitStmt(Stmt *S) {
    if (isa<WhileStmt>(S) || isa<ForStmt>(S) || isa<DoStmt>(S))
      Loops.push_back(S);

    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (Loops.empty() || Functions.empty()) 
      return false;

    // Randomly select a loop and a function
    Stmt *selectedLoop = randElement(Loops);
    FunctionDecl *selectedFunc = randElement(Functions);

    // Find insertion point
    Stmt *loopBody;
    if (auto WS = dyn_cast<WhileStmt>(selectedLoop)) {
      loopBody = WS->getBody();
    } else if (auto FS = dyn_cast<ForStmt>(selectedLoop)) {
      loopBody = FS->getBody();
    } else {  // DoStmt
      loopBody = cast<DoStmt>(selectedLoop)->getBody();
    }

    if (!loopBody) 
      return false;

    // Refactor the loop body into a compound statement if it is not already
    std::string loopBodyText = getSourceText(loopBody);
    if (!isa<CompoundStmt>(loopBody)) {
      loopBodyText = "{\n" + loopBodyText + "\n}";
      getRewriter().ReplaceText(loopBody->getSourceRange(), loopBodyText);
    }

    // Insert function call at the start of the loop body
    SourceLocation insertLoc = findStringLocationFrom(loopBody->getBeginLoc(), "{").getLocWithOffset(1);
    std::string funcCall = selectedFunc->getNameAsString() + "();\n";
    getRewriter().InsertTextAfter(insertLoc, funcCall);

    return true;
  }

private:
  std::vector<Stmt *> Loops;
  std::vector<FunctionDecl *> Functions;
};

static RegisterMutator<InsertFunctionCallInLoop> M("u3.InsertFunctionCallInLoop",
    "Selects a loop in a function and inserts a call to a random function at the start of the loop body");
