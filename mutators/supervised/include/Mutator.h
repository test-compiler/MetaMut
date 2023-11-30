#ifndef YSMUT_MUTATOR_H
#define YSMUT_MUTATOR_H

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Lexer.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/ADT/SmallPtrSet.h>

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string>
#include <utility>

#include "json.hpp"
using json = nlohmann::ordered_json;

namespace ysmut {

class MutatorManager;

template <typename, typename = void>
struct has_getSourceRange : std::false_type {};
template <typename T>
struct has_getSourceRange<T,
    std::void_t<decltype(std::declval<T>().getSourceRange())>>
    : std::true_type {};

struct MutationRange {
  enum : unsigned { npos = UINT_MAX };
  std::string filename;
  unsigned beginOffset = 0;
  unsigned endOffset = npos; // [0, UINT_MAX]

  bool isValid() const { return true; }

  static MutationRange from_json(json j) {
    MutationRange range;
    range.beginOffset = j[0];
    range.endOffset = j[1];
    assert(range.beginOffset <= range.endOffset);
    return range;
  }

  json to_json() const {
    json j = json::object();
    j["filename"] = filename;
    j["location"] = json::array({beginOffset, endOffset});
    return j;
  }

  clang::SourceRange toClangSourceRange(clang::SourceManager &SM) const {
    auto fileEntry = SM.getFileManager().getFile(filename);
    if (!fileEntry) return clang::SourceRange();
    unsigned beginOffset = this->beginOffset;
    unsigned endOffset = this->endOffset;

    clang::FileID file = SM.translateFile(fileEntry.get());

    // Check if endOffset exceeds file size.
    unsigned fileSize = SM.getFileIDSize(file);
    if (beginOffset >= fileSize) { beginOffset = fileSize - 1; }
    if (endOffset >= fileSize) { endOffset = fileSize - 1; }

    clang::SourceLocation beginLoc = SM.getComposedLoc(file, beginOffset);
    clang::SourceLocation endLoc = SM.getComposedLoc(file, endOffset);

    return clang::SourceRange(beginLoc, endLoc);
  }
};

inline bool rangesIntersect(
    clang::SourceManager &SM, clang::SourceRange r1, clang::SourceRange r2) {
  unsigned r1Begin = SM.getFileOffset(r1.getBegin());
  unsigned r1End = SM.getFileOffset(r1.getEnd());
  unsigned r2Begin = SM.getFileOffset(r2.getBegin());
  unsigned r2End = SM.getFileOffset(r2.getEnd());
  return (r1Begin <= r2End) && (r2Begin <= r1End);
}

class Mutator {
  const char *name;
  const char *desc;
  MutatorManager *manager = nullptr;

public:
  Mutator(const char *name, const char *desc) : name(name), desc(desc) {}
  Mutator(const Mutator &) = delete;
  Mutator(Mutator &&) = default;
  Mutator &operator=(const Mutator &) = delete;
  Mutator &operator=(Mutator &&) = default;

  clang::ASTContext &getASTContext() const;
  clang::CompilerInstance &getCompilerInstance() const;
  MutationRange getMutationRange() const;

  clang::SourceRange getMutationSourceRange() const {
    auto &SM = getASTContext().getSourceManager();
    return getMutationRange().toClangSourceRange(SM);
  }

  void setManager(MutatorManager *manager) { this->manager = manager; }
  MutatorManager &getManager() const {
    assert(this->manager);
    return *(this->manager);
  }

  clang::Decl *getMostRecentTranslationUnitDecl(clang::FunctionDecl *FD);

  std::string formatAsDecl(clang::QualType ty, const std::string &placeholder);

  clang::SourceRange getExpansionRange(clang::SourceRange range);
  clang::SourceLocation getLocForEndOfToken(clang::SourceLocation loc);
  clang::SourceLocation findStringLocationFrom(
      clang::SourceLocation beginLoc, llvm::StringRef target);
  clang::SourceRange findBracesRange(clang::SourceLocation beginLoc,
      char beginBrace = '(', char endBrace = ')');

  clang::SourceLocation getDeclEndLoc(clang::Decl *decl);

  template <class NodeType>
  llvm::StringRef getSourceText(const NodeType *node) {
    clang::SourceRange range = node->getSourceRange();
    auto &SM = getASTContext().getSourceManager();
    auto &LO = getASTContext().getLangOpts();
    range = SM.getExpansionRange(range).getAsRange();
    const char *beg = SM.getCharacterData(range.getBegin());
    clang::SourceLocation E = range.getEnd();
    E = clang::Lexer::getLocForEndOfToken(E, 0, SM, LO);
    const char *end = SM.getCharacterData(E);
    return llvm::StringRef(beg, end - beg);
  }

  clang::Rewriter &getRewriter() const;

  template <class NodeType>
  bool isMutationSite(const NodeType *node) {
    auto &SM = getASTContext().getSourceManager();
    auto B_is_macro = node->getBeginLoc().isMacroID();
    auto E_is_macro = node->getBeginLoc().isMacroID();
    return !B_is_macro && !E_is_macro && SM.isInMainFile(node->getBeginLoc()) &&
           rangesIntersect(
               SM, getMutationSourceRange(), node->getSourceRange());
  }

  llvm::raw_ostream &outs();

  // for random
  unsigned randIndex(unsigned n);
  bool randBool();

  template <typename Container>
  auto &randElement(Container &container) {
    assert(!container.empty());
    return container[randIndex(container.size())];
  }

  std::string generateUniqueName(const std::string &baseName);

  // return true if successfully mutate
  virtual bool mutate() = 0;

  virtual ~Mutator() = default;
};

} // namespace ysmut

#endif
