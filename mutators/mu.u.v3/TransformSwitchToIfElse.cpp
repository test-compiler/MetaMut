#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class TransformSwitchToIfElse : public Mutator,
                                public clang::RecursiveASTVisitor<TransformSwitchToIfElse> {
public:
    using Mutator::Mutator;

    bool VisitSwitchStmt(clang::SwitchStmt *SS) {
        SwitchStmts.push_back(SS);
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (SwitchStmts.empty()) return false;

        SwitchStmt *stmt = randElement(SwitchStmts);
        std::string newIfElseStmt = createIfElseEquivalent(stmt);
        getRewriter().ReplaceText(stmt->getSourceRange(), newIfElseStmt);
        return true;
    }

private:
    std::vector<clang::SwitchStmt *> SwitchStmts;

    std::string createIfElseEquivalent(clang::SwitchStmt *stmt) {
        std::string conditionVar = getSourceText(stmt->getCond());
        std::string newStmt;
        std::string conditions;
        std::string defaultStmt;
        bool isFallThrough = false;

        for (SwitchCase *sc = stmt->getSwitchCaseList(); sc; sc = sc->getNextSwitchCase()) {
            Stmt *subStmt = sc->getSubStmt();
            if (isa<CaseStmt>(sc)) {
                CaseStmt *caseStmt = cast<CaseStmt>(sc);
                if (!conditions.empty()) conditions += " || ";
                conditions += conditionVar + " == " + getSourceText(caseStmt->getLHS());

                if (dyn_cast<NullStmt>(subStmt)) {
                    isFallThrough = true;
                } else {
                    newStmt += "if (" + conditions + ") {\n" + getSubStatements(subStmt) + "}\n";
                    conditions.clear();
                    isFallThrough = false;
                }
            } else if (isa<DefaultStmt>(sc)) {
                if (!conditions.empty() && !isFallThrough) {
                    newStmt += "if (" + conditions + ") {\n" + getSubStatements(subStmt) + "}\n";
                    conditions.clear();
                }
                defaultStmt = "else {\n" + getSubStatements(subStmt) + "}\n";
            }
        }

        newStmt += defaultStmt;

        return newStmt;
    }

    std::string getSubStatements(Stmt *stmt) {
        std::string subStmts;
        if (CompoundStmt *CS = dyn_cast<CompoundStmt>(stmt)) {
            for (Stmt *s : CS->body()) {
                subStmts += "\t" + getSourceText(s) + ";\n";
            }
        } else {
            subStmts += "\t" + getSourceText(stmt) + ";\n";
        }
        return subStmts;
    }
};

static RegisterMutator<TransformSwitchToIfElse> M("u3.TransformSwitchToIfElse", "Change 'switch' statement to equivalent 'if-else' statements.");
