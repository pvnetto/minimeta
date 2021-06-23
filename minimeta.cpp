// =============================================-----
// ======== Entry point
// =============================================-----

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/Refactoring.h"

#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"


using namespace llvm;
using namespace clang::tooling;

static llvm::cl::OptionCategory s_MinimetaCategory {"minimeta options"};
static cl::extrahelp s_CommonHelp{ CommonOptionsParser::HelpMessage};
static cl::extrahelp s_MoreHelp {"\nFor more info: github.com/pvnetto\n"};


using namespace clang;
using namespace clang::ast_matchers;

// Finds all annotated types
auto s_SerializableTypeMatcher = 
    cxxRecordDecl(
        decl().bind("id"),
        hasAttr(attr::Annotate)
    );

bool IsSerializableField(FieldDecl* const fieldDecl) {
    if(auto attr = fieldDecl->getAttr<clang::AnnotateAttr>()) {
        // Attributes annotated with mm-add are serialized
        if(attr->getAnnotation() == "mm-add")
            return true;
        // Attributes annotated with mm-ignore are not serialized
        if(attr->getAnnotation() == "mm-ignore")
            return false;
    }
    
    // Only public fields are serialized, unless annotations state otherwise    
    return fieldDecl->getAccess() == AccessSpecifier::AS_public;
}

void FindSerializableFields(const CXXRecordDecl* typeDecl) {
    if(typeDecl->isClass()) {
        printf("type => class\n");
    }
    else if(typeDecl->isStruct()) {
        printf("type => struct\n");
    }
    
    printf("name => %s", typeDecl->getNameAsString().c_str());
    printf("\n");
    int i = 0;
    for(const auto& field : typeDecl->fields()) {
        if(i++ == 0) {
            printf("====== SERIALIZABLE FIELDS ======\n");
        }
        if(IsSerializableField(field)) {
            printf("name => %s\n", field->getNameAsString().c_str());
            printf("type => ");
            field->getType().dump();
            printf("\n");
        }
    }

    printf("\n\n");
}

// Action that gets called when a matcher finds something
class TypeMetaGenerator : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult& result) {
    if(const CXXRecordDecl* typeDecl = result.Nodes.getNodeAs<clang::CXXRecordDecl>("id")) {
        printf("===== Found serializable type! =====\n");

        if(auto attr = typeDecl->getAttr<clang::AnnotateAttr>()) {
            if(attr->getAnnotation() == "mm-type") {
                FindSerializableFields(typeDecl);
            }
        }
    }
  }
};

int main(int argc, const char** argv) {
  auto expectedParser =
      CommonOptionsParser::create(argc, argv, s_MinimetaCategory);

  if (!expectedParser) {
    llvm::errs() << expectedParser.takeError();
    return 1;
  }

  CommonOptionsParser &optionsParser = expectedParser.get();
  RefactoringTool tool{ optionsParser.getCompilations(), optionsParser.getSourcePathList() };

  MatchFinder matchFinder;
  TypeMetaGenerator generator;
  matchFinder.addMatcher(s_SerializableTypeMatcher, &generator);

  //tool.run(newFrontendActionFactory(&matchFinder).get());         // Runs but doesn't apply
  tool.runAndSave(newFrontendActionFactory(&matchFinder).get());    // Runs and applies
}
