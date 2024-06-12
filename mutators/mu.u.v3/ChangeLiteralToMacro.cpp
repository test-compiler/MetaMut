#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Expr.h>

using namespace clang;

class ChangeLiteralToMacro : public Mutator,
                      public clang::RecursiveASTVisitor<ChangeLiteralToMacro> {
public:
  using Mutator::Mutator;

  bool VisitIntegerLiteral(IntegerLiteral *IL) {
    literals.push_back(IL);
    return true;
  }

  bool mutate() override {
    TraverseDecl(getASTContext().getTranslationUnitDecl());

    if (literals.empty()) {
      return false;
    }

    IntegerLiteral *literal = randElement(literals);
    int value = *literal->getValue().getRawData();
    std::string macroName = "INT_LITERAL_" + std::to_string(value);
    
    // Get the location of the start of the main file
    FileID mainFileID = getASTContext().getSourceManager().getMainFileID();
    SourceLocation startLoc = getASTContext().getSourceManager().getLocForStartOfFile(mainFileID);

    getRewriter().InsertText(startLoc, "#define " + macroName + " " + std::to_string(value) + "\n");
    getRewriter().ReplaceText(literal->getSourceRange(), macroName);

    return true;
  }

private:
  std::vector<IntegerLiteral *> literals;
};

static RegisterMutator<ChangeLiteralToMacro> M("u3.ChangeLiteralToMacro", "Replaces an integer literal with a predefined macro representing the same value.");
