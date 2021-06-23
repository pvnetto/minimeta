// ====================================================-----
// ======== Entry point
// ====================================================-----

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace llvm;
using namespace clang::tooling;

static llvm::cl::OptionCategory s_MinimetaCategory{"minimeta options"};
static cl::extrahelp s_CommonHelp{CommonOptionsParser::HelpMessage};
static cl::extrahelp s_MoreHelp{"\nFor more info: github.com/pvnetto\n"};

using namespace clang;
using namespace clang::ast_matchers;

namespace mmeta {
struct FieldInfo {
  std::string Name, Type;

  void Dump() const {
    printf("name => %s, type => %s\n", Name.c_str(), Type.c_str());
  }
};

struct TypeInfo {
  std::string Name, Type;
  std::string Filename;
  std::vector<FieldInfo> Fields;

  void Dump() const {
    printf("# Serializable type:\n");
    printf("name => %s\n", Name.c_str());
    printf("type => %s\n", Type.c_str());
    printf("filename: %s\n", Filename.c_str());
    printf("\n");

    if (Fields.size() > 0)
      printf("## Fields:\n");

    for (const auto &field : Fields) {
      field.Dump();
    }
    printf("\n");
  }
};
} // namespace mmeta

void GenerateTypeMetadataSource(std::vector<mmeta::TypeInfo> &types) {
  for (const auto &metaType : types) {
    metaType.Dump();
  }
}

bool IsSerializableField(FieldDecl *const fieldDecl) {
  if (auto attr = fieldDecl->getAttr<clang::AnnotateAttr>()) {
    // Attributes annotated with mm-add are serialized
    if (attr->getAnnotation() == "mm-add")
      return true;
    // Attributes annotated with mm-ignore are not serialized
    if (attr->getAnnotation() == "mm-ignore")
      return false;
  }

  // Only public fields are serialized, unless annotations state otherwise
  return fieldDecl->getAccess() == AccessSpecifier::AS_public;
}

std::vector<mmeta::FieldInfo>
FindSerializableFields(const CXXRecordDecl *typeDecl) {
  std::vector<mmeta::FieldInfo> fields;
  for (const auto &field : typeDecl->fields()) {
    if (IsSerializableField(field)) {
      mmeta::FieldInfo metaField;
      metaField.Name = field->getNameAsString();
      metaField.Type = field->getType().getAsString();
      fields.push_back(metaField);
    }
  }
  return fields;
}

// Action that gets called when a matcher finds something
class TypeMetaGenerator : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &result) override {
    SourceManager &sourceManager = result.Context->getSourceManager();

    if (const CXXRecordDecl *typeDecl =
            result.Nodes.getNodeAs<clang::CXXRecordDecl>("id")) {
      if (auto attr = typeDecl->getAttr<clang::AnnotateAttr>()) {
        if (attr->getAnnotation() == "mm-type") {
          mmeta::TypeInfo metaType;

          metaType.Name = typeDecl->getNameAsString();

          metaType.Filename =
              sourceManager.getFilename(typeDecl->getLocation()).str();
          metaType.Filename = metaType.Filename.substr(
              0,
              metaType.Filename.find_first_of('.')); // Removes file extension
          metaType.Filename = metaType.Filename.append(".generated.hxx");

          if (typeDecl->isClass()) {
            metaType.Type = "class";
          }
          if (typeDecl->isStruct()) {
            metaType.Type = "struct";
          }

          metaType.Fields = FindSerializableFields(typeDecl);
          m_TypeMetadata.push_back(metaType);
        }
      }
    }
  }

  virtual void onEndOfTranslationUnit() override {
      printf("Found %i types in translation unit:\n", (int) m_TypeMetadata.size());
    GenerateTypeMetadataSource(m_TypeMetadata);
  }

private:
  std::vector<mmeta::TypeInfo> m_TypeMetadata;
};

int main(int argc, const char **argv) {
  auto expectedParser =
      CommonOptionsParser::create(argc, argv, s_MinimetaCategory);

  if (!expectedParser) {
    llvm::errs() << expectedParser.takeError();
    return 1;
  }

  CommonOptionsParser &optionsParser = expectedParser.get();
  RefactoringTool tool{optionsParser.getCompilations(),
                       optionsParser.getSourcePathList()};

  // Finds all annotated types
  auto serializableTypeMatcher =
      cxxRecordDecl(decl().bind("id"), hasAttr(attr::Annotate));

  MatchFinder matchFinder;
  TypeMetaGenerator generator;
  matchFinder.addMatcher(serializableTypeMatcher, &generator);

  tool.runAndSave(newFrontendActionFactory(&matchFinder).get());
}
