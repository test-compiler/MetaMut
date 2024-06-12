#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>

using namespace clang;

class RemoveRandomStructField : public Mutator,
                                public clang::RecursiveASTVisitor<RemoveRandomStructField> {
public:
  using Mutator::Mutator;

  bool VisitRecordDecl(RecordDecl *RD) {
    if (RD->isStruct()) {
      TheStructs.push_back(RD);
    }
    return true;
  }

  bool VisitMemberExpr(MemberExpr *ME) {
    TheMemberExprs.push_back(ME);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheStructs.empty()) return false;

    RecordDecl *chosenStruct = randElement(TheStructs);
    FieldDecl *fieldToRemove = nullptr;
    std::vector<MemberExpr *> exprToRemove;

    for (auto it = chosenStruct->field_begin(); it != chosenStruct->field_end(); ++it) {
      if (randBool()) {
        fieldToRemove = *it;
        break;
      }
    }

    if (fieldToRemove) {
      for (auto *ME : TheMemberExprs) {
        if (ME->getMemberDecl() == fieldToRemove) {
          exprToRemove.push_back(ME);
        }
      }

      for (auto *ME : exprToRemove) {
        // replace with a default value, here we use zero for simplicity
        getRewriter().ReplaceText(ME->getSourceRange(), "0 /* field removed */");
      }

      getRewriter().RemoveText(fieldToRemove->getSourceRange());
      return true;
    }

    return false;
  }

private:
  std::vector<RecordDecl *> TheStructs;
  std::vector<MemberExpr *> TheMemberExprs;
};

static RegisterMutator<RemoveRandomStructField> M("u3.RemoveRandomStructField", "This mutator selects a structure definition in the code and removes a random field from it and replace its usages with a default value.");
