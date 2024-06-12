// ReorderStructFields.cpp
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>
#include <algorithm>

#include "MutatorManager.h"

using namespace clang;

class ReorderStructFields : public Mutator,
                      public clang::RecursiveASTVisitor<ReorderStructFields> {

public:
  using Mutator::Mutator;
  
  bool VisitRecordDecl(RecordDecl *RD) {
    if (RD->isStruct()) {
      TheStructs.push_back(RD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheStructs.empty()) return false;

    RecordDecl *structDecl = randElement(TheStructs);

    std::vector<FieldDecl*> fields(structDecl->field_begin(), structDecl->field_end());
    std::shuffle(fields.begin(), fields.end(), getManager().getRandomGenerator());

    std::string newStruct = "struct { ";
    for (auto field : fields) {
        newStruct += getSourceText(field) + "; ";
    }
    newStruct += "}";
    
    getRewriter().ReplaceText(structDecl->getSourceRange(), newStruct);
    
    return true;
  }

private:
  std::vector<RecordDecl *> TheStructs;
};

static RegisterMutator<ReorderStructFields> M("u3.ReorderStructFields", "Selects a structure definition and reorders its fields randomly");
