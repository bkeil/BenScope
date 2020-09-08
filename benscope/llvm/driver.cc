#include <iostream>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "absl/strings/str_join.h"
#include "benscope/llvm/KaleidoscopeJIT.h"
#include "benscope/llvm/codegen.h"
#include "benscope/llvm/environment.h"
#include "benscope/parsing/ast.h"
#include "benscope/parsing/lexer.h"
#include "benscope/parsing/parser.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

namespace benscope {
namespace {

void InitializeModuleAndPassManager(
    const Environment &environment, const llvm::orc::KaleidoscopeJIT &jit,
    llvm::StringRef module_name, std::unique_ptr<llvm::Module> *module,
    std::unique_ptr<llvm::legacy::FunctionPassManager> *fpm) {
  *module = std::make_unique<llvm::Module>(module_name, *environment.context);
  (**module).setDataLayout(jit.getTargetMachine().createDataLayout());

  *fpm = std::make_unique<llvm::legacy::FunctionPassManager>(module->get());
  (**fpm).add(llvm::createInstructionCombiningPass());
  (**fpm).add(llvm::createReassociatePass());
  (**fpm).add(llvm::createGVNPass());
  (**fpm).add(llvm::createCFGSimplificationPass());
  (**fpm).doInitialization();
}

void CompileFunction(Environment *environment, llvm::orc::KaleidoscopeJIT *jit,
                     const FunctionAST &func) {
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::legacy::FunctionPassManager> fpm;
  InitializeModuleAndPassManager(*environment, *jit, func.proto->name, &module,
                                 &fpm);

  llvm::errs() << "Created module (" << module->getName() << ")\n";

  environment->module = module.get();
  auto f = llvm::dyn_cast_or_null<llvm::Function>(
      ValueVisitor::ValueOf(func, environment));
  environment->module = nullptr;
  if (!f) {
    std::cerr << "Error in compiling function.\n";
    return;
  }

  std::cerr << "Function definition:\n";
  f->print(llvm::errs());

  std::cerr << "Function verification: " << llvm::verifyFunction(*f) << "\n";

  fpm->run(*f);

  std::cerr << "Function optimized to:\n";
  f->print(llvm::errs());


  jit->addModule(std::move(module));
}

void CompileExtern(Environment *environment, llvm::orc::KaleidoscopeJIT *jit,
                   const PrototypeAST &proto) {
  std::string m_name = "__extern_";
  m_name.append(proto.name);
  std::unique_ptr<llvm::Module> module =
      std::make_unique<llvm::Module>(m_name, *environment->context);

  environment->RegisterProto(proto);

  environment->module = module.get();
  auto f = llvm::dyn_cast_or_null<llvm::Function>(
      ValueVisitor::ValueOf(proto, environment));
  environment->module = nullptr;
  if (!f) {
    std::cerr << "Error in compiling extern.\n";
    return;
  }

  std::cerr << "Extern declaration:\n";
  f->print(llvm::errs());

  jit->addModule(std::move(module));
  std::cerr << "Declaration added to JIT.\n";
}

void ExecuteFunction(Environment *environment, llvm::orc::KaleidoscopeJIT *jit,
                     const FunctionAST &func) {
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::legacy::FunctionPassManager> fpm;

  InitializeModuleAndPassManager(*environment, *jit, "_anon_module", &module, &fpm);

  environment->module = module.get();
  auto f = llvm::dyn_cast_or_null<llvm::Function>(
      ValueVisitor::ValueOf(func, environment));
  environment->module = nullptr;
  if (!f) {
    std::cerr << "Error in compiling expression.\n";
    return;
  }

  std::cerr << "Optimized anonymous function to:\n";
  llvm::verifyFunction(*f, &llvm::errs());
  fpm->run(*f);
  f->print(llvm::errs());

  auto anon_module_key = jit->addModule(std::move(module));
  auto ExprSymbol = jit->findSymbol(std::string(kAnonExpr));
  if (ExprSymbol) {
    llvm::Expected<llvm::JITTargetAddress> address = ExprSymbol.getAddress();
    if (!address) {
      std::cerr << "Unable to get address for anonymous function.\n";
      llvm::Error error = address.takeError();
      std::cerr << "Error: ";
      llvm::errs() << error << "\n";
    } else {
      // Get the symbol's address and cast it to the right type (takes no
      // arguments, returns a double) so we can call it as a native function.
      double (*FP)() =
          (double (*)())(intptr_t)llvm::cantFail(ExprSymbol.getAddress());
      std::cout << "Evaluated to " << FP() << "\n";
    }
  } else {
    std::cerr << "Anonymous function symbol can't be found. Sad Trombone.";
  }

  // Delete the anonymous expression module from the JIT.
  jit->removeModule(anon_module_key);
  std::cerr << "Anonymous function removed from JIT.\n";
}

void MainLoop(Environment *environment, llvm::orc::KaleidoscopeJIT *jit,
              Parser *parser) {
  while (!parser->eof()) {
    std::unique_ptr<AST> ast = parser->ParseNext();

    if (!ast) {
      std::cerr << "Trying to recover from error.\n";
      parser->GetNextToken();
      continue;
    }

    AST *statement = ast.get();
    if (auto *f_ast = dynamic_cast<FunctionAST *>(statement)) {
      if (f_ast->proto->name == kAnonExpr) {
        ExecuteFunction(environment, jit, *f_ast);
      } else {
        CompileFunction(environment, jit, *f_ast);
      }
    } else if (auto *p_ast = dynamic_cast<PrototypeAST *>(statement)) {
      CompileExtern(environment, jit, *p_ast);
    } else {
      llvm::Value *value = ValueVisitor::ValueOf(*ast, environment);
      std::cerr << "Unexpected AST.";
      if (value) {
        value->print(llvm::errs());
      } else {
        std::cerr << "No value.";
      }
    }
  }
}

} // namespace
} // namespace benscope

int main() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  auto jit = std::make_unique<llvm::orc::KaleidoscopeJIT>();

  llvm::LLVMContext context;
  llvm::IRBuilder<> builder(context);
  absl::flat_hash_map<std::string, benscope::PrototypeAST> function_protos;

  benscope::Environment environment;
  environment.context = &context;
  environment.builder = &builder;
  environment.function_protos = &function_protos;

  while (!std::cin.eof()) {
    std::stringstream input;
    std::string line;
    std::cout << "\nBenScope> ";
    std::getline(std::cin, line);

    input << line;
    auto lexer = std::make_unique<benscope::Lexer>(&input);
    benscope::Parser parser(std::move(lexer));
    benscope::MainLoop(&environment, jit.get(), &parser);
  }

  return 0;
}