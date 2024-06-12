#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class NegateIfCondition : public Mutator,
                          public clang::RecursiveASTVisitor<NegateIfCondition> {
public:
    explicit NegateIfCondition(const char *name, const char *desc)
        : Mutator(name, desc) {}

    bool VisitIfStmt(clang::IfStmt *IS) {
        TheIfStmts.push_back(IS);
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheIfStmts.empty()) return false;

        clang::IfStmt *SelectedStmt = randElement(TheIfStmts);

        Expr *Condition = SelectedStmt->getCond();
        SourceLocation Begin = Condition->getBeginLoc();
        SourceLocation End = Condition->getEndLoc();

        std::string NewCondition = "!(" + getSourceText(Condition) + ")";
        getRewriter().ReplaceText(SourceRange(Begin, End), NewCondition);

        return true;
    }

private:
    std::vector<clang::IfStmt *> TheIfStmts;
};

static RegisterMutator<NegateIfCondition> M("u3.NegateIfCondition", "Selects an 'if' statement in the code and negates the condition, effectively changing the control flow path of the program.");
