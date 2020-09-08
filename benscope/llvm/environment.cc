
#include <string>

#include "absl/container/flat_hash_map.h"
#include "benscope/llvm/environment.h"
#include "benscope/parsing/ast.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

namespace benscope {

llvm::Value *Environment::Lookup(std::string_view name) {
  if (named_values.contains(name)) {
    return named_values[name];
  } else if (parent) {
    return parent->Lookup(name);
  } else {
    return nullptr;
  }
}

const PrototypeAST &Environment::RegisterProto(const PrototypeAST &prototype) {
  (*function_protos)[prototype.name] = prototype;
  return prototype;
}

llvm::Function *Environment::LookupFunction(const std::string &name) {
  // First, see if the function has already been added to the current module.
  assert(module && "Module null");
  if (auto *f = module->getFunction(name))
    return f;

  // If not, check whether we can codegen the declaration from some existing
  // prototype.
  auto proto_it = function_protos->find(name);
  if (proto_it != function_protos->end())
    return CompileProto(proto_it->second);

  // If no existing prototype exists, return null.
  return nullptr;
}

llvm::Function *Environment::CompileProto(const PrototypeAST &proto) {
  // Make the function type:  double(double,double) etc.
  std::vector<llvm::Type *> doubles(proto.args.size(),
                                    llvm::Type::getDoubleTy(*context));
  llvm::FunctionType *f_type = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(*context), doubles, false);

  llvm::errs() << "Declaring function (" << proto.name << ") in module ("
               << module->getName() << ").\n";
  llvm::Function *f = llvm::Function::Create(
      f_type, llvm::Function::ExternalLinkage, proto.name, module);

  // Set names for all arguments.
  unsigned idx = 0;
  for (auto &arg : f->args())
    arg.setName(proto.args[idx++]);

  return f;
}

const PrototypeAST *Environment::LookupProto(std::string_view name) {
  if (function_protos->contains(name)) {
    return &function_protos->at(name);
  } else {
    return nullptr;
  }
}

Environment Environment::Spawn() {
  Environment e;
  e.builder = builder;
  e.context = context;
  e.module = module;
  e.function_protos = function_protos;
  e.parent = this;
  return e;
}

} // namespace benscope
