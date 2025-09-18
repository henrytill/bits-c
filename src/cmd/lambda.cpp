#include <iostream>
#include <memory>
#include <string>

struct Exprvisitor;

struct Expr {
	virtual ~Expr() = default;
	virtual void accept(Exprvisitor &v) = 0;
};

using Exprptr = std::unique_ptr<Expr>;

struct Var : Expr {
	std::string name;

	explicit Var(std::string n)
		: name{std::move(n)}
	{
	}

	operator Exprptr() &&
	{
		return std::make_unique<Var>(std::move(name));
	}

	void accept(Exprvisitor &v) override;
};

struct Lam : Expr {
	std::string param;
	Exprptr body;

	Lam(std::string p, Exprptr b)
		: param{std::move(p)}
		, body{std::move(b)}
	{
	}

	operator Exprptr() &&
	{
		return std::make_unique<Lam>(std::move(param), std::move(body));
	}

	void accept(Exprvisitor &v) override;
};

struct App : Expr {
	Exprptr fun;
	Exprptr arg;

	App(Exprptr f, Exprptr a)
		: fun{std::move(f)}
		, arg{std::move(a)}
	{
	}

	operator Exprptr() &&
	{
		return std::make_unique<App>(std::move(fun), std::move(arg));
	}

	void accept(Exprvisitor &v) override;
};

struct Exprvisitor {
	virtual void visit(Var &e) = 0;
	virtual void visit(Lam &e) = 0;
	virtual void visit(App &e) = 0;
};

void
Var::accept(Exprvisitor &v)
{
	v.visit(*this);
}

void
Lam::accept(Exprvisitor &v)
{
	v.visit(*this);
}

void
App::accept(Exprvisitor &v)
{
	v.visit(*this);
}

class Exprshow : public Exprvisitor {
	std::ostream &out;

public:
	explicit Exprshow(std::ostream &out)
		: out{out}
	{
	}

	void
	visit(Var &e) override
	{
		out << e.name;
	}

	void
	visit(Lam &e) override
	{
		out << "(\\" << e.param;
		out << " . ";
		e.body->accept(*this);
		out << ")";
	}

	void
	visit(App &e) override
	{
		out << "(";
		e.fun->accept(*this);
		out << " ";
		e.arg->accept(*this);
		out << ")";
	}

	void
	operator()(Expr &e)
	{
		e.accept(*this);
	}
};

auto
main(void) -> int
{
	auto show = Exprshow(std::cout);

	// (\x . x)
	auto id = Lam("x", Var("x"));

	show(id);
	std::cout << std::endl;

	// (\t . (\f . t))
	auto k = Lam("t", Lam("f", Var("t")));

	show(k);
	std::cout << std::endl;

	// ((\x . x) y)
	auto app1 = App(std::move(id), Var("y"));

	show(app1);
	std::cout << std::endl;

	return 0;
}
