#ifndef __BENSCOPE_PARSING_CODEGEN_H__
#define __BENSCOPE_PARSING_CODEGEN_H__

#include <iostream>

#include "benscope/llvm/environment.h"
#include "benscope/parsing/ast.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Type.h"

namespace benscope {

class ValueVisitor : public AstVisitor {
public:
  static llvm::Value *ValueOf(const AST &ast, Environment *environment) {
    ValueVisitor vv(environment);
    ast.Accept(vv);
    return vv.value_;
  }

  explicit ValueVisitor(Environment *environment)
      : environment_(environment), value_(nullptr) {}

  void Visit(const BinaryExprAST &expr) override {
    llvm::Value *l = GetValue(*expr.lhs), *r = GetValue(*expr.rhs);
    if (!l || !r)
      return;

    llvm::IRBuilder<> &builder = *environment_->builder;
    switch (expr.op) {
    case '+':
      value_ = builder.CreateFAdd(l, r, "addtmp");
      break;
    case '-':
      value_ = builder.CreateFSub(l, r, "subtmp");
      break;
    case '*':
      value_ = builder.CreateFMul(l, r, "multmp");
      break;
    case '/':
      value_ = builder.CreateFDiv(l, r, "divtmp");
      break;
    case '<': {
      llvm::Value *test = builder.CreateFCmpULT(l, r, "cmptmp");
      value_ = builder.CreateUIToFP(
          test, llvm::Type::getDoubleTy(*environment_->context), "booltmp");
      break;
    }
    default:
      break;
    }
  }

  void Visit(const CallExprAST &expr) override {
    llvm::Function *callee = environment_->LookupFunction(expr.callee);

    if (!callee) {
      std::cerr << "Unknown function " << expr.callee << "\n";
      return;
    }

    std::cerr << "Found function (" << expr.callee << "): ";
    callee->print(llvm::errs());

    if (callee->arg_size() != expr.args.size()) {
      std::cerr << "Wrong number of arguments to " << expr.callee << "\n";
      return;
    }

    std::vector<llvm::Value *> args;
    for (auto &argExpr : expr.args) {
      llvm::Value *v = GetValue(*argExpr);
      if (!v)
        return;
      args.push_back(v);
    }

    value_ = environment_->builder->CreateCall(callee, args, "calltmp");
  }

  void Visit(const IfExprAST &expr) override {
    llvm::Value *cond = GetValue(*expr.test);
    if (!cond) {
      std::cerr << "Error in compiling test of if-expression.\n";
      return;
    }

    llvm::LLVMContext &context = *environment_->context;
    llvm::IRBuilder<> &builder = *environment_->builder;

    // Convert to bool.
    cond = builder.CreateFCmpONE(
        cond, llvm::ConstantFP::get(context, llvm::APFloat(0.0)), "iftest");
    llvm::Function *parent = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *if_true =
        llvm::BasicBlock::Create(context, "if_true", parent);
    llvm::BasicBlock *if_false = llvm::BasicBlock::Create(context, "if_false");
    llvm::BasicBlock *if_end = llvm::BasicBlock::Create(context, "if_end");

    builder.CreateCondBr(cond, if_true, if_false);

    builder.SetInsertPoint(if_true);
    llvm::Value* if_true_v = GetValue(*expr.if_true);
    if (!if_true_v) {
      std::cerr << "Error compiling if-true branch of if-expression.";
      return;
    }
    builder.CreateBr(if_end);
    if_true = builder.GetInsertBlock();

    parent->getBasicBlockList().push_back(if_false);
    builder.SetInsertPoint(if_false);
    llvm::Value* if_false_v = GetValue(*expr.if_false);
    if (!if_false_v) {
      std::cerr << "Error compiling if-true branch of if-expression.";
      return;
    }
    builder.CreateBr(if_end);
    if_false = builder.GetInsertBlock();

    parent->getBasicBlockList().push_back(if_end);
    builder.SetInsertPoint(if_end);
    llvm::PHINode *pn = builder.CreatePHI(llvm::Type::getDoubleTy(context), 2, "iftmp");
    pn->addIncoming(if_true_v, if_true);
    pn->addIncoming(if_false_v, if_false);
    value_ = pn;
  }

  void Visit(const NumberExprAST &expr) override {
    value_ =
        llvm::ConstantFP::get(*environment_->context, llvm::APFloat(expr.val));
  };

  void Visit(const VariableExprAST &expr) override {
    llvm::Value *v = environment_->Lookup(expr.name_);
    if (!v) {
      std::cerr << "Unknown variable " << expr.name_ << "\n";
    } else {
      value_ = v;
    }
  };

  void Visit(const FunctionAST &ast) override {
    auto &proto = environment_->RegisterProto(*ast.proto);
    llvm::Function *f = environment_->LookupFunction(proto.name);
    if (!f) {
      value_ = nullptr;
      return;
    }

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *bb =
        llvm::BasicBlock::Create(*environment_->context, "entry", f);
    environment_->builder->SetInsertPoint(bb);

    // Record the function arguments in the NamedValues map.
    Environment f_env = environment_->Spawn();

    for (auto &arg : f->args())
      f_env.named_values[arg.getName()] = &arg;

    if (llvm::Value *ret_val = GetValue(*ast.body, &f_env)) {
      // Finish off the function.
      environment_->builder->CreateRet(ret_val);
      llvm::errs() << "Defining function (" << f->getName() << ") in module ("
                   << environment_->module->getName() << ")\n";
      value_ = f;
      return;
    }

    // Error reading body, remove function.
    llvm::errs() << "Error defining function.  Erasing from parent.\n";
    f->eraseFromParent();
    value_ = nullptr;
  };

  void Visit(const PrototypeAST &ast) override {
    value_ = environment_->CompileProto(ast);
  };

private:
  llvm::Value *GetValue(AST &ast, Environment *env = nullptr) {
    if (env == nullptr)
      env = environment_;
    ValueVisitor child(env);
    ast.Accept(child);
    return child.value_;
  }

  Environment *environment_;
  llvm::Value *value_;
};

} // namespace benscope

#endif // __BENSCOPE_PARSING_CODEGEN_H__