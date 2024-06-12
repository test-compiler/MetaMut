#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceNestedLoopsWithRecursion : public Mutator,
                                        public RecursiveASTVisitor<ReplaceNestedLoopsWithRecursion> {
public:
  using Mutator::Mutator;

  bool VisitStmt(Stmt *S) {
    if (auto *FS = dyn_cast<ForStmt>(S)) {
      if (auto *CS = dyn_cast<CompoundStmt>(FS->getBody())) {
        for (auto *InnerStmt : CS->children()) {
          if (auto *InnerFS = dyn_cast<ForStmt>(InnerStmt)) {
            LoopInstances.push_back(std::make_tuple(currentFunction, FS, InnerFS));
          }
        }
      }
    }
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    currentFunction = FD;
    TraverseStmt(FD->getBody());
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (LoopInstances.empty()) return false;

    auto selectedLoops = randElement(LoopInstances);

    // Extract body of the inner loop
    std::string innerLoopBody = getSourceText(std::get<2>(selectedLoops)->getBody());

    // Extract initial declarations, increment expressions, and conditions from the loops
    std::string outerLoopInit = getSourceText(std::get<1>(selectedLoops)->getInit());
    std::string outerLoopCond = getSourceText(std::get<1>(selectedLoops)->getCond());
    std::string outerLoopInc = getSourceText(std::get<1>(selectedLoops)->getInc());
    std::string innerLoopInit = getSourceText(std::get<2>(selectedLoops)->getInit());
    std::string innerLoopCond = getSourceText(std::get<2>(selectedLoops)->getCond());
    std::string innerLoopInc = getSourceText(std::get<2>(selectedLoops)->getInc());

    // Generate a unique function name to avoid conflicts
    std::string funcName = generateUniqueName("thisFunc");

    // Create a recursive function body that embeds the inner loop's body
    std::string funcBody = "{ i = 0; j = 0; if(" + outerLoopCond + " && " + innerLoopCond + ") { "
                         + innerLoopBody + " " + funcName + "(arr, " + outerLoopInc + ", " + innerLoopInc + "); } }";

    // Create a function declaration and definition with the recursive body
    std::string funcDeclaration = "void " + funcName + "(int** arr, int i, int j) " + funcBody;

    // add the function declaration before the enclosing function declaration
    if(!addStringBeforeFunctionDecl(std::get<0>(selectedLoops), funcDeclaration)) return false;

    // Replace the outer loop with a call to the recursive function
    getRewriter().ReplaceText(std::get<1>(selectedLoops)->getSourceRange(),
                              funcName + "(&arr, 0, 0);");

    return true;
  }

private:
  FunctionDecl *currentFunction = nullptr;
  std::vector<std::tuple<FunctionDecl *, ForStmt *, ForStmt *>> LoopInstances;
};

static RegisterMutator<ReplaceNestedLoopsWithRecursion> M("u3.ReplaceNestedLoopsWithRecursion",
    "Replaces a pair of nested loops with an equivalent recursive function");
