#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class ModifyLoopIncrement : public Mutator, 
                            public clang::RecursiveASTVisitor<ModifyLoopIncrement> {
public:
    using Mutator::Mutator;
    using RecursiveASTVisitor<ModifyLoopIncrement>::TraverseStmt;

    bool VisitForStmt(ForStmt* forStmt) {
        if (auto* unaryOp = dyn_cast<UnaryOperator>(forStmt->getInc())) {
            if (unaryOp->getOpcode() == UO_PreInc || unaryOp->getOpcode() == UO_PostInc) {
                if (auto* expr = dyn_cast<DeclRefExpr>(unaryOp->getSubExpr()->IgnoreParenImpCasts())) {
                    if (expr->getDecl()->getType()->isIntegerType()) {
                        LoopIncrements.push_back(forStmt->getInc());
                    }
                }
            }
        }
        return true;
    }

    bool VisitWhileStmt(WhileStmt* whileStmt) {
        if (auto* body = dyn_cast<CompoundStmt>(whileStmt->getBody())) {
            for (auto it = body->body_begin(), end = body->body_end(); it != end; ++it) {
                if (auto* unaryOp = dyn_cast<UnaryOperator>(*it)) {
                    if (unaryOp->getOpcode() == UO_PreInc || unaryOp->getOpcode() == UO_PostInc) {
                        if (auto* expr = dyn_cast<DeclRefExpr>(unaryOp->getSubExpr()->IgnoreParenImpCasts())) {
                            if (expr->getDecl()->getType()->isIntegerType()) {
                                LoopIncrements.push_back(unaryOp);
                            }
                        }
                    }
                }
            }
        }
        return true;
    }

    bool mutate() override {
        LoopIncrements.clear();
        for (auto it = getASTContext().getTranslationUnitDecl()->decls_begin();
             it != getASTContext().getTranslationUnitDecl()->decls_end(); ++it) {
            if (auto* FD = dyn_cast<FunctionDecl>(*it)) {
                if (Stmt* Body = FD->getBody()) {
                    TraverseStmt(Body);
                }
            }
        }

        if (LoopIncrements.empty()) {
            return false;
        }

        Expr* incrementExpr = randElement(LoopIncrements);
        UnaryOperator* unaryExpr = dyn_cast<UnaryOperator>(incrementExpr);

        std::string newIncExpr;
        if (randBool()) {
            newIncExpr = getSourceText(unaryExpr->getSubExpr()) + " += 2"; 
        } else {
            newIncExpr = getSourceText(unaryExpr->getSubExpr()) + " -= 1"; 
        }
        getRewriter().ReplaceText(incrementExpr->getSourceRange(), newIncExpr);

        return true;
    }

private:
    std::vector<Expr*> LoopIncrements;
};

static RegisterMutator<ModifyLoopIncrement> M("u3.ModifyLoopIncrement", "Modifies increment statement of a loop");
