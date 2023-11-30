#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class RandomizeCaseStmts: public Mutator, public RecursiveASTVisitor<RandomizeCaseStmts> {
public:
    using Mutator::Mutator;
    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheSwitchStmts.empty()) return false;

        SwitchStmt *stmt = randElement(TheSwitchStmts);

        // Get the CaseStmts in the SwitchStmt and shuffle their order
        std::vector<std::string> caseStmts;
        CompoundStmt *compoundStmt = dyn_cast<CompoundStmt>(stmt->getBody());

        if (compoundStmt == nullptr || compoundStmt->size() == 0) return false;

        for (auto stmtIt = compoundStmt->body_begin(); stmtIt != compoundStmt->body_end(); ++stmtIt) {
            std::string stmtText;
            while (stmtIt != compoundStmt->body_end() && !isa<CaseStmt>(*stmtIt) && !isa<DefaultStmt>(*stmtIt)) {
                stmtText += getSourceText(*stmtIt).str() + "\n";
                ++stmtIt;
            }
            if (!stmtText.empty()) {
                caseStmts.push_back(stmtText);
            }
        }

        if (caseStmts.size() < 2) return false; // do not shuffle if there is only one case

        std::shuffle(caseStmts.begin(), caseStmts.end(), getManager().getRandomGenerator());

        // Build a new CompoundStmt with the shuffled CaseStmts
        std::string newCompoundStmtStr = "{\n";
        for (const auto &caseStmt : caseStmts) {
            newCompoundStmtStr += caseStmt;
        }
        newCompoundStmtStr += "}\n";

        // Replace the original CompoundStmt with the new one
        getRewriter().ReplaceText(compoundStmt->getSourceRange(), newCompoundStmtStr);

        return true;
    }

    bool VisitSwitchStmt(clang::SwitchStmt *SS) {
        TheSwitchStmts.push_back(SS);
        return true;
    }

private:
    std::vector<clang::SwitchStmt *> TheSwitchStmts;
};

static RegisterMutator<RandomizeCaseStmts> M("RandomizeCaseStmts", "Randomly shuffles the order of CaseStmts in a SwitchStmt");