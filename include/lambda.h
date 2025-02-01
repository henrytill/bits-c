#pragma once

#include <iostream>
#include <memory>
#include <string>

// References:
// ===========
// https://github.com/keean/Compositional-Typing-Inference/blob/ded90f8a38d59c93c9eb269517d42b5350d81fe2/infer.cpp
// https://arne-mertz.de/2016/04/visitor-pattern-oop/
// https://stackoverflow.com/questions/8114276/how-do-i-pass-a-unique-ptr-argument-to-a-constructor-or-a-function
// https://codereview.stackexchange.com/questions/100855/bst-c-stl-implementation-visiting-algorithms

struct ExprVisitor;

// Lambda Expressions
struct Expr {
  virtual ~Expr() {}
  virtual void accept(ExprVisitor &v) = 0;
};

struct Var : Expr {
  explicit Var(std::string n) : name(n) {}

  std::string name;
  void accept(ExprVisitor &v) override;
};

struct Abs : Expr {
  explicit Abs(std::string n, const Var &v)
      : name(n), body(new Var(v)) {}
  explicit Abs(std::string n, std::unique_ptr<Expr> b)
      : name(n), body(std::move(b)) {}

  std::string name;
  std::unique_ptr<Expr> body;
  void accept(ExprVisitor &v) override;
};

struct App : Expr {
  explicit App(std::unique_ptr<Expr> f, const Var &v)
      : fun(std::move(f)), arg(new Var(v)) {}
  explicit App(std::unique_ptr<Expr> f, std::unique_ptr<Expr> a)
      : fun(std::move(f)), arg(std::move(a)) {}

  std::unique_ptr<Expr> fun;
  std::unique_ptr<Expr> arg;
  void accept(ExprVisitor &v) override;
};

// Expr Visitor
struct ExprVisitor {
  virtual void visit(Var &e) = 0;
  virtual void visit(Abs &e) = 0;
  virtual void visit(App &e) = 0;
};

void Var::accept(ExprVisitor &v) { v.visit(*this); }

void Abs::accept(ExprVisitor &v) { v.visit(*this); }

void App::accept(ExprVisitor &v) { v.visit(*this); }

// Printing Lambda Expressions
class ExprShow : public ExprVisitor {
  std::ostream &out;

public:
  explicit ExprShow(std::ostream &out) : out(out) {}

  void visit(Var &e) override { out << e.name; }

  void visit(Abs &e) override {
    out << "(\\" << e.name;
    out << " . ";
    e.body->accept(*this);
    out << ")";
  }

  void visit(App &e) override {
    out << "(";
    e.fun->accept(*this);
    out << " ";
    e.arg->accept(*this);
    out << ")";
  }

  void operator()(Expr &e) { e.accept(*this); }
};

// Local Variables:
// mode: c++
// End:
//
// vim: set filetype=cpp:
