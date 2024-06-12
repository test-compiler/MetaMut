#include "Mutator.h"
#include "MutatorManager.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"

using namespace clang;

class SwapSwitchCases : public Mutator, 
                        public RecursiveASTVisitor<SwapSwitchCases> {

public:
  using Mutator::Mutator;

  bool VisitSwitchStmt(SwitchStmt *stmt) {
    SwitchStmts.push_back(stmt);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (SwitchStmts.empty())
      return false;

    auto switchStmt = randElement(SwitchStmts);
    auto body = cast<CompoundStmt>(switchStmt->getBody());

    std::vector<CaseStmt*> caseStmts;
    for (auto stmt : body->body()) {
      if (CaseStmt* caseStmt = dyn_cast<CaseStmt>(stmt))
        caseStmts.push_back(caseStmt);
    }

    if (caseStmts.size() < 2)
      return false;

    unsigned index1 = randIndex(caseStmts.size());
    unsigned index2 = index1;
    while (index2 == index1) { // ensure different indices
      index2 = randIndex(caseStmts.size());
    }

    // swapping statements
    auto caseText1 = Lexer::getSourceText(CharSourceRange::getTokenRange(caseStmts[index1]->getSourceRange()), getASTContext().getSourceManager(), getASTContext().getLangOpts()).str();
    auto caseText2 = Lexer::getSourceText(CharSourceRange::getTokenRange(caseStmts[index2]->getSourceRange()), getASTContext().getSourceManager(), getASTContext().getLangOpts()).str();

    getRewriter().ReplaceText(caseStmts[index1]->getSourceRange(), caseText2);
    getRewriter().ReplaceText(caseStmts[index2]->getSourceRange(), caseText1);

    return true;
  }

private:
  std::vector<SwitchStmt*> SwitchStmts;
};

static RegisterMutator<SwapSwitchCases> M("u3.SwapSwitchCases",
    "Swaps two random case branches within a switch statement");
