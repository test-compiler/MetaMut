#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class AddRandomGotoStmt : public Mutator,
                          public clang::RecursiveASTVisitor<AddRandomGotoStmt> {

  using VisitorTy = clang::RecursiveASTVisitor<AddRandomGotoStmt>;

public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLabels.empty() || TheStmts.empty()) return false;

    LabelStmt *label = randElement(TheLabels);
    Stmt *stmt = randElement(TheStmts);

    std::string labelName = label->getName();
    std::string gotoStmt = "goto " + labelName + ";";

    SourceRange stmtRange = getExpansionRange(stmt->getSourceRange());
    std::string stmtText = getSourceText(stmt).str();

    getRewriter().ReplaceText(stmtRange, stmtText + gotoStmt);

    return true;
  }

  bool VisitLabelStmt(LabelStmt *LS) {
    TheLabels.push_back(LS);
    return true;
  }

  bool VisitCompoundStmt(CompoundStmt *CS) {
    for (auto it = CS->body_begin(), et = CS->body_end(); it != et; ++it) {
        if (std::next(it) != et) { // Exclude the last statement
            TheStmts.push_back(*it);
        }
    }
    return true;
  }

private:
  std::vector<LabelStmt *> TheLabels;
  std::vector<Stmt *> TheStmts;
};

static RegisterMutator<AddRandomGotoStmt> X(
    "AddRandomGotoStmt", "Insert a new GotoStmt at a random position within a CompoundStmt excluding the last statement");