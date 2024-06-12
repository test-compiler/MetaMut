#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceVarWithExternalData : public Mutator,
                                   public RecursiveASTVisitor<ReplaceVarWithExternalData> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    if(VD->isLocalVarDecl() && VD->getDeclContext()->isFunctionOrMethod() && VD->getInit()) {
      VarDecls.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if(VarDecls.empty()) return false;

    VarDecl *VD = randElement(VarDecls);
    FunctionDecl *FD = cast<FunctionDecl>(VD->getDeclContext());

    std::string fileName = generateUniqueName("datafile");
    std::string varName = VD->getNameAsString();
  
    std::string newCode = "FILE* " + varName + "_file = fopen(\"" + fileName + "\", \"r\");\n";
    newCode += VD->getType().getAsString() + " " + varName + ";\n";
    newCode += "fread(&" + varName + ", sizeof(" + VD->getType().getAsString() + "), 1, " + varName + "_file);\n";
    newCode += "fclose(" + varName + "_file);\n";
  
    getRewriter().ReplaceText(VD->getSourceRange(), newCode);
    
    return true;
  }

private:
  std::vector<VarDecl *> VarDecls;
};

static RegisterMutator<ReplaceVarWithExternalData> M("u3.ReplaceVarWithExternalData", "Replace a variable with data read from an external file.");
