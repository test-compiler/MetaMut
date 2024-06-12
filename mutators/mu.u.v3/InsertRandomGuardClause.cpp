#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertRandomGuardClause : public Mutator,
                      public clang::RecursiveASTVisitor<InsertRandomGuardClause> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (FD->hasBody() && FD->doesThisDeclarationHaveABody() && FD->isFirstDecl() && FD->getNameInfo().getAsString() != "main") {
      TheFunctions.push_back(FD);
    }
    return true;
  }
  
  bool VisitCallExpr(CallExpr *CE) {
    FunctionDecl *FD = CE->getDirectCallee();
    if(FD && std::find(TheFunctions.begin(), TheFunctions.end(), FD) != TheFunctions.end()) {
      CallsToUpdate[FD].push_back(CE);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheFunctions.empty()) return false;
    
    FunctionDecl *FD = randElement(TheFunctions);
    
    std::string paramName = generateUniqueName("guard");
    std::string paramDeclaration = "int " + paramName;
    
    std::string commaIfNeeded = FD->param_empty() ? "" : ", ";
    paramDeclaration = commaIfNeeded + paramDeclaration;
    
    SourceRange paramListRange = findBracesRange(FD->getBeginLoc(), '(', ')');
    getRewriter().InsertTextBefore(paramListRange.getEnd(), paramDeclaration);

    std::string guardClause;
    if (FD->getReturnType().getAsString() == "void") {
      guardClause = "if(" + paramName + ") return;\n";
    } else {
      guardClause = "if(" + paramName + ") return 0;\n";
    }

    Stmt *funcBody = FD->getBody();
    if (CompoundStmt *CS = dyn_cast<CompoundStmt>(funcBody)) {
      getRewriter().InsertTextAfter(CS->getLBracLoc().getLocWithOffset(1), guardClause);
    }

    // Update calls
    for(CallExpr *CE : CallsToUpdate[FD]) {
      getRewriter().InsertTextBefore(CE->getRParenLoc(), ", 0");
    }

    return true;
  }

private:
  std::vector<FunctionDecl *> TheFunctions;
  std::unordered_map<FunctionDecl*, std::vector<CallExpr*>> CallsToUpdate;
};

static RegisterMutator<InsertRandomGuardClause> M("u3.InsertRandomGuardClause", "This mutator selects a random function in the code and inserts a guard clause (early return statement) at the beginning of the function based on a newly introduced parameter.");
