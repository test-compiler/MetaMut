#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class SplitLoopCondition : public Mutator,
                      public clang::RecursiveASTVisitor<SplitLoopCondition> {
public:
  using Mutator::Mutator;

  bool VisitForStmt(ForStmt* FS) {
    if (auto *BO = dyn_cast<BinaryOperator>(FS->getCond()))
        if (BO->getOpcode() == BO_LAnd || BO->getOpcode() == BO_LOr)
            TheLoops.push_back(FS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLoops.empty()) return false;

    ForStmt *Loop = randElement(TheLoops);
    BinaryOperator *Cond = cast<BinaryOperator>(Loop->getCond());
    Stmt* Body = Loop->getBody();

    // Split the condition
    StringRef Condition = Lexer::getSourceText(CharSourceRange::getTokenRange(Cond->getSourceRange()),
                                 getASTContext().getSourceManager(), LangOptions(), nullptr);

    SmallVector<StringRef, 2> Conditions;
    Condition.split(Conditions, (Cond->getOpcode() == BO_LAnd) ? "&&" : "||");

    // Create the new nested loop
    std::string NewLoop = "for (" + getSourceText(Loop->getInit()) + "; " 
                                  + Conditions[0].str() + "; "
                                  + getSourceText(Loop->getInc()) + ") { "
                                  + "for (; " + Conditions[1].str() + ";) { "
                                  + getSourceText(Body) + " } }";

    // Replace the old loop with the new one
    getRewriter().ReplaceText(Loop->getSourceRange(), NewLoop);

    return true;
  }

private:
  std::vector<clang::ForStmt *> TheLoops;
};

static RegisterMutator<SplitLoopCondition> M("u3.SplitLoopCondition", "Splits loop conditions into separate nested loops.");
