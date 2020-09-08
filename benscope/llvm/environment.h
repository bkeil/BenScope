#ifndef __BENSCOPE_PARSING_ENVIRONMENT_H__
#define __BENSCOPE_PARSING_ENVIRONMENT_H__

#include <string>

#include "absl/container/flat_hash_map.h"
#include "benscope/parsing/ast.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

namespace benscope {

struct Environment {
  Environment *parent;

  llvm::IRBuilder<> *builder;
  llvm::LLVMContext *context;
  llvm::Module *module;
  absl::flat_hash_map<std::string, PrototypeAST> *function_protos;

  absl::flat_hash_map<std::string, llvm::Value *> named_values;

  // Look up variable bindings.
  llvm::Value *Lookup(std::string_view name);

  // Register a function prototype with the environment.  New registrations
  // overwrite previous ones.
  const PrototypeAST &RegisterProto(const PrototypeAST &prototype);

  // Returns the most recently registered prototype for the given function name.
  const PrototypeAST *LookupProto(std::string_view name);

  // Returns the function definition in the current module, if it exists,
  // otherwise, if a prototype exists for the function, registers an extern in
  // the current module for that function.  Returns null if there is neither a
  // current function nor a prototype.
  llvm::Function *LookupFunction(const std::string& name);

  // Records the prototype in the current module, as an externally linked
  // function.
  llvm::Function *CompileProto(const PrototypeAST &proto);

  // Spans an child environment with a new variable binding scope.  Bindings in
  // this environment are visible in the child, but same-name bindings in the
  // child will shadow them.
  Environment Spawn();
};

} // namespace benscope

#endif // __BENSCOPE_PARSING_ENVIRONMENT_H__