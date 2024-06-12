// InsertRandomParameterAssignment.cpp
#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class InsertRandomParameterAssignment : public Mutator,
                      public clang::RecursiveASTVisitor<InsertRandomParameterAssignment> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (FD->doesThisDeclarationHaveABody() && FD->getNumParams() > 0) {
      TheFunctions.push_back(FD);
    }
    return true;
  }

  bool mutate() override {
    TraverseDecl(getASTContext().getTranslationUnitDecl());

    if (TheFunctions.empty()) return false;

    FunctionDecl *FD = randElement(TheFunctions);
    ParmVarDecl* Param = FD->getParamDecl(randIndex(FD->getNumParams()));
    std::string ParamName = Param->getName().str();
    std::string InitialValue;

    if (Param->getType()->isIntegerType()) {
      InitialValue = "0";
    } else if (Param->getType()->isPointerType()) {
      InitialValue = "NULL";
    } else {
      // If not a handled type, we don't perform a mutation
      return false;
    }

    std::string NewAssignment = ParamName + " = " + InitialValue + ";\n";
    
    // Find location immediately after the opening brace of the function body
    SourceLocation LocAfterOpeningBrace = FD->getBody()->getBeginLoc().getLocWithOffset(1);

    // Insert the new assignment after the opening brace of the function body
    getRewriter().InsertText(LocAfterOpeningBrace, NewAssignment, true, true);

    return true;
  }

private:
  std::vector<clang::FunctionDecl *> TheFunctions;
};

static RegisterMutator<InsertRandomParameterAssignment> M("u3.InsertRandomParameterAssignment", "Inserts a random assignment to a parameter at the start of a function body");
