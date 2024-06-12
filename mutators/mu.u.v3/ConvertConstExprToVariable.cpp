#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class ConvertConstExprToVariable : public Mutator, public RecursiveASTVisitor<ConvertConstExprToVariable> {
public:
    using Mutator::Mutator;

    bool VisitBinaryOperator(BinaryOperator* BinOp) {
        if (BinOp->isIntegerConstantExpr(getASTContext())) {
            constExprs.push_back(std::make_pair(BinOp, currentFunction));
        }
        return true;
    }

    bool TraverseFunctionDecl(FunctionDecl* FD) {
        currentFunction = FD;
        return RecursiveASTVisitor<ConvertConstExprToVariable>::TraverseFunctionDecl(FD);
    }

    bool mutate() override {
        TraverseAST(getASTContext());

        if (constExprs.empty())
            return false;

        auto pairToReplace = randElement(constExprs);
        BinaryOperator* exprToReplace = pairToReplace.first;
        FunctionDecl* functionOfExpr = pairToReplace.second;

        std::string exprText = getSourceText(exprToReplace);

        // Create new variable initialization
        std::string varName = generateUniqueName("constantReplaced");
        std::string varDeclText = "int " + varName + " = " + exprText + ";";

        // Insert new variable initialization before the function
        addStringBeforeFunctionDecl(functionOfExpr, varDeclText);

        // Replace constant expression with variable name
        getRewriter().ReplaceText(exprToReplace->getSourceRange(), varName);

        return true;
    }

private:
    FunctionDecl* currentFunction = nullptr;
    std::vector<std::pair<BinaryOperator*, FunctionDecl*>> constExprs;
};

static RegisterMutator<ConvertConstExprToVariable> M("u3.ConvertConstExprToVariable", "Replace a constant expression with a newly defined variable of the same value.");
