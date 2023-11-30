#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class SwapNestedLoops
    : public Mutator,
      public clang::RecursiveASTVisitor<SwapNestedLoops> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (OuterLoops.empty() || InnerLoops.empty()) return false;

    // Select a random loop pair to swap
    unsigned idx = randIndex(OuterLoops.size());
    Stmt *outerLoop = OuterLoops[idx];
    Stmt *innerLoop = InnerLoops[idx];

    // Extract the bodies of the outer and inner loops
    Stmt *outerLoopBody = getLoopBody(outerLoop);
    Stmt *innerLoopBody = getLoopBody(innerLoop);

    if (!outerLoopBody || !innerLoopBody) return false;

    // Store the source texts of the inner and outer loop bodies before performing any replacements
    std::string innerLoopBodyText = getSourceText(innerLoopBody).str();
    std::string outerLoopBodyText = getSourceText(outerLoopBody).str();
    
    // Placeholder for the inner loop
    std::string placeholder = "// placeholder for inner loop";

    // Replace the inner loop with the placeholder
    getRewriter().ReplaceText(innerLoop->getSourceRange(), placeholder);

    // Replace the outer loop body with the inner loop body
    getRewriter().ReplaceText(outerLoopBody->getSourceRange(), innerLoopBodyText);

    // Find the location of the placeholder and replace it with the outer loop body
    SourceLocation placeholderLoc = getASTContext().getSourceManager().translateFileLineCol(
        getASTContext().getSourceManager().getFileEntryForID(getASTContext().getSourceManager().getMainFileID()), 1, 1);
    getRewriter().ReplaceText(placeholderLoc, placeholder.size(), outerLoopBodyText);

    return true;
  }

  bool TraverseStmt(Stmt *S) {
    if (!S) return RecursiveASTVisitor::TraverseStmt(S);

    if (isa<ForStmt>(S) || isa<WhileStmt>(S)) {
      currentLoop = S;
    }

    bool ret = RecursiveASTVisitor::TraverseStmt(S);

    if (S == currentLoop) {
      currentLoop = nullptr;
    }

    return ret;
  }

  bool VisitStmt(Stmt *S) {
    if (currentLoop && (isa<ForStmt>(S) || isa<WhileStmt>(S))) {
      // We found a nested loop
      OuterLoops.push_back(currentLoop);
      InnerLoops.push_back(S);
    }
    return true;
  }

private:
  Stmt *getLoopBody(Stmt *loop) {
    if (auto *forLoop = dyn_cast<ForStmt>(loop)) {
      return forLoop->getBody();
    } else if (auto *whileLoop = dyn_cast<WhileStmt>(loop)) {
      return whileLoop->getBody();
    }
    return nullptr;
  }

  Stmt *currentLoop = nullptr;
  std::vector<Stmt *> OuterLoops;
  std::vector<Stmt *> InnerLoops;
};

static RegisterMutator<SwapNestedLoops> X("SwapNestedLoops", "Swap the bodies of inner and outer loop.");