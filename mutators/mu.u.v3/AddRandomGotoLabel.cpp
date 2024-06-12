#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class AddRandomGotoLabel : public Mutator,
                           public clang::RecursiveASTVisitor<AddRandomGotoLabel> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (FD->hasBody()) {
      CompoundStmt *CS = dyn_cast<CompoundStmt>(FD->getBody());
      if(CS) TheFunctions.push_back(std::make_tuple(FD, CS->body_begin(), CS->body_end()));
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFunctions.empty()) return false;

    auto selectedFunction = randElement(TheFunctions);

    // Generate unique label name
    std::string labelName = generateUniqueName("label");

    // Generate a goto statement
    std::string gotoStmt = "goto " + labelName + ";\n";

    // Add a goto statement at the start of function body
    addStringAfterStmt(*std::get<1>(selectedFunction), gotoStmt);

    // Generate a label statement
    std::string labelStmt = labelName + ":;\n";

    // Add a label at random index in function body
    unsigned int stmtCount = std::distance(std::get<1>(selectedFunction), std::get<2>(selectedFunction));
    unsigned int randIndex = getManager().getRandomGenerator()() % stmtCount;
    Stmt *stmt = *(std::get<1>(selectedFunction) + randIndex);
    addStringAfterStmt(stmt, labelStmt);

    return true;
  }

private:
  std::vector<std::tuple<FunctionDecl*, CompoundStmt::body_iterator, CompoundStmt::body_iterator>> TheFunctions;
};

static RegisterMutator<AddRandomGotoLabel> M("u3.AddRandomGotoLabel", "Introduce a new 'goto' statement that jumps to a randomly placed label within the function.");
