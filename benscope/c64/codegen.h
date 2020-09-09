#ifndef __BENSCOPE_C64_CODEGEN_H__
#define __BENSCOPE_C64_CODEGEN_H__

#include <cstdint>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "benscope/c64/cbm_floats.h"
#include "benscope/parsing/ast.h"

namespace benscope::c64 {

struct Scope {
  using VTable = absl::flat_hash_map<std::string, int>;

  int offset = 0;
  VTable v_table;
};

class CodeGen : public AstVisitor {
public:
  void Visit(const BinaryExprAST &expr) override;
  void Visit(const CallExprAST &expr) override;
  void Visit(const IfExprAST &expr) override;
  void Visit(const NumberExprAST &expr) override;
  void Visit(const VariableExprAST &expr) override;

  void Visit(const FunctionAST &ast) override;
  void Visit(const PrototypeAST &ast) override;

  std::string_view ToStringView();
  const std::string &ToString();

private:
  using VarSet = absl::flat_hash_set<std::string>;

  void Line(int depth, std::string_view text);

  VarSet _globals;
  VarSet _zp_globals;
  Scope *_scope;

  std::string _buffer;
};

} // namespace benscope::c64

#endif // __BENSCOPE_C64_CODEGEN_H__