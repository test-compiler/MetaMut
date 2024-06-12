#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReverseArrayInitialization : public Mutator,
                                   public clang::RecursiveASTVisitor<ReverseArrayInitialization> {
public:
  using Mutator::Mutator;

  bool VisitInitListExpr(InitListExpr *ILE) {
    TheInitLists.push_back(ILE);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheInitLists.empty()) return false;

    InitListExpr *ILE = randElement(TheInitLists);
    
    std::vector<std::string> InitValues;
    for (unsigned int i = 0; i < ILE->getNumInits(); i++) {
        InitValues.push_back(getSourceText(ILE->getInit(i)));
    }
    std::reverse(InitValues.begin(), InitValues.end());

    std::string ReversedInitList = "{ " + join(InitValues, ", ") + " }";
    getRewriter().ReplaceText(ILE->getSourceRange(), ReversedInitList);

    return true;
  }

private:
  std::vector<InitListExpr*> TheInitLists;

  std::string join(const std::vector<std::string>& vec, const char* delim) {
    std::ostringstream oss;
    copy(vec.begin(), vec.end()-1, std::ostream_iterator<std::string>(oss, delim));
    oss << vec.back();  // Add the last element without delimiter
    return oss.str();
  }
};

static RegisterMutator<ReverseArrayInitialization> M("u3.ReverseArrayInitialization", "This mutator selects an array initialization and reverses the order of initialization elements.");
