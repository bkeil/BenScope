#include "benscope/c64/codegen.h"

namespace benscope::c64 {
namespace {
template <typename T> struct reversing_wrapper { T &iterable; };

template <typename T> auto begin(reversing_wrapper<T> w) {
  return std::rbegin(w.iterable);
}

template <typename T> auto end(reversing_wrapper<T> w) {
  return std::rend(w.iterable);
}

template <typename T> reversing_wrapper<T> reverse(T &&iterable) {
  return {iterable};
}

std::string mangle(std::string_view name) { return absl::StrCat("_", name); }
} // namespace

void CodeGen::Visit(const BinaryExprAST &expr) {
  expr.rhs->Accept(*this);
  Line(1, "jsr push_fac");
  expr.lhs->Accept(*this);
  Line(1, "ldy sp + 1");
  Line(1, "lda sp");
  switch (expr.op) {
  case '+': {
    Line(1, "jsr FADD");
    break;
  }
  case '-': {
    Line(1, "jsr FSUB");
    break;
  }
  default: { Line(1, "brk"); }
  }
}

void CodeGen::Visit(const CallExprAST &expr) {
  Line(1, absl::StrCat("; Call ", expr.callee));
  const int offset = _scope->offset;
  int argc = 0;
  for (const auto &arg : expr.args) {
    Line(2, absl::StrCat("; Push arg ", ++argc));
    arg->Accept(*this);
    Line(1, "jsr push_fac");
    _scope->offset += 5;
  }
  Line(2, absl::StrCat("; Perform call"));
  Line(1, absl::StrCat("jsr ", mangle(expr.callee)));
  _scope->offset = offset;
}

void CodeGen::Visit(const IfExprAST &expr) {
  Line(0, ".scope ifexpr");
  expr.test->Accept(*this);
  Line(1, "lda FAC_EXPONENT");
  Line(1, "bne false");
  expr.if_true->Accept(*this);
  Line(1, "jmp done");
  Line(0, "false:");
  expr.if_false->Accept(*this);
  Line(0, "done:");
  Line(0, ".endscope");
}

void CodeGen::Visit(const NumberExprAST &expr) {
  CBMUnpackedFloat val(expr.val);
  Line(1, absl::StrCat("; FAC = ", expr.val));
  Line(1, absl::StrCat("lda #$", absl::Hex(val.exponent_excess_128)));
  Line(1, "sta FAC_EXPONENT");
  Line(1, absl::StrCat("lda #$", absl::Hex(val.mantissa_0)));
  Line(1, "sta FAC_MANTISSA0");
  Line(1, absl::StrCat("lda #$", absl::Hex(val.mantissa_1)));
  Line(1, "sta FAC_MANTISSA1");
  Line(1, absl::StrCat("lda #$", absl::Hex(val.mantissa_2)));
  Line(1, "sta FAC_MANTISSA2");
  Line(1, absl::StrCat("lda #$", absl::Hex(val.mantissa_3)));
  Line(1, "sta FAC_MANTISSA3");
  Line(1, absl::StrCat("lda #$", absl::Hex(val.sign)));
  Line(1, "sta FAC_SIGN");
  Line(1, "lda #$0");
  Line(1, "sta FAC_ROUNDING");
  Line(0, "");
}

void CodeGen::Visit(const VariableExprAST &expr) {
  Line(1, absl::StrCat("lda #", _scope->v_table[expr.name_], " + ",
                       _scope->offset));
  Line(1, "jsr sp_plus_a_to_ya");
  Line(1, "jsr BASIC_LoadFAC");

  _globals.insert("sp_plus_a_to_ya");
}

void CodeGen::Visit(const FunctionAST &ast) {
  Line(0, absl::StrCat(".proc ", mangle(ast.proto->name), ": near"));
  int stack = 0;
  Scope scope;
  _scope = &scope;
  for (const auto &arg : reverse(ast.proto->args)) {
    scope.v_table[arg] = stack;
    stack += 5;
  }
  for (const auto &entry : scope.v_table) {
    Line(1, absl::StrCat("; ", entry.first, " := ", entry.second));
  }
  ast.body->Accept(*this);
  _scope = nullptr;

  Line(1, absl::StrCat("lda #", stack));
  Line(1, "jsr sp_plus_a_to_ya");
  Line(1, "sty sp + 1");
  Line(1, "sta sp");
  Line(0, absl::StrCat(".endproc"));
}

void CodeGen::Visit(const PrototypeAST &ast) { _globals.insert(ast.name); }

void CodeGen::Line(int depth, std::string_view text) {
  for (int i = 0; i < depth * 4; ++i)
    _buffer.push_back(' ');
  absl::StrAppend(&_buffer, text);
  _buffer.push_back('\n');
}

std::string_view CodeGen::ToStringView() { return _buffer; }
const std::string &CodeGen::ToString() { return _buffer; }

} // namespace benscope::c64