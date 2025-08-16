#pragma once

#include <iostream>
#include <memory>
#include <string>

struct expr_visitor;

struct expr {
    virtual ~expr() = default;
    virtual void accept(expr_visitor &v) = 0;
};

using expr_ptr = std::unique_ptr<expr>;

struct var : expr {
    std::string name;

    explicit var(std::string n) : name{std::move(n)} {}

    operator expr_ptr() && {
        return std::make_unique<var>(std::move(name));
    }

    void accept(expr_visitor &v) override;
};

struct lam : expr {
    std::string param;
    expr_ptr body;

    lam(std::string p, expr_ptr b)
        : param{std::move(p)}, body{std::move(b)} {}

    operator expr_ptr() && {
        return std::make_unique<lam>(std::move(param), std::move(body));
    }

    void accept(expr_visitor &v) override;
};

struct app : expr {
    expr_ptr fun;
    expr_ptr arg;

    app(expr_ptr f, expr_ptr a)
        : fun{std::move(f)}, arg{std::move(a)} {}

    operator expr_ptr() && {
        return std::make_unique<app>(std::move(fun), std::move(arg));
    }

    void accept(expr_visitor &v) override;
};

struct expr_visitor {
    virtual void visit(var &e) = 0;
    virtual void visit(lam &e) = 0;
    virtual void visit(app &e) = 0;
};

void var::accept(expr_visitor &v) { v.visit(*this); }

void lam::accept(expr_visitor &v) { v.visit(*this); }

void app::accept(expr_visitor &v) { v.visit(*this); }

class expr_show : public expr_visitor {
    std::ostream &out;

public:
    explicit expr_show(std::ostream &out) : out{out} {}

    void visit(var &e) override { out << e.name; }

    void visit(lam &e) override {
        out << "(\\" << e.param;
        out << " . ";
        e.body->accept(*this);
        out << ")";
    }

    void visit(app &e) override {
        out << "(";
        e.fun->accept(*this);
        out << " ";
        e.arg->accept(*this);
        out << ")";
    }

    void operator()(expr &e) { e.accept(*this); }
};

// Local Variables:
// mode: c++
// End:
//
// vim: set filetype=cpp:
