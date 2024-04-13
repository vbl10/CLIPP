#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <sstream>
#include <list>


class option_base
{
public:
    const std::vector<std::string> aliases;
    const std::string desc;
    const bool require;
    option_base(std::vector<std::string>&& aliases, const std::string& desc, bool require = false)
        :aliases(std::move(aliases)), desc(desc), require(require)
    {

    }
    virtual std::istream& parse(std::istream& in) = 0;
    virtual const char* get_typename() const { return ""; };
};

template <typename type>
class option : public option_base
{
public:
    type val;

    option(std::vector<std::string>&& aliases, type default_val, const std::string& desc, bool require = false)
        :option_base(std::move(aliases), desc, require), val(default_val)
    {

    }

    std::istream& parse(std::istream& in) override
    {
        in >> val;
        return in;
    }

    const char* get_typename() const override
    {
        return typeid(type).name();
    }
};
template <>
class option<bool> : public option_base
{
public:
    bool val;

    option(std::vector<std::string>&& aliases, bool default_val, const std::string& desc, bool require = false)
        :option_base(std::move(aliases), desc, require), val(default_val)
    {

    }

    std::istream& parse(std::istream& in) override
    {
        val = true;
        return in;
    }
};

typedef std::vector<option_base*> cmd_args;
class terminal {
private:
	typedef std::function<void(const cmd_args&)> func_cmd;
	class nodo {
	public:
		const std::string desc;

		nodo(const std::string& manual) :desc(manual) {}

		virtual int operator()(const std::string& rotulo, terminal& raiz) const = 0;
		virtual const std::map<std::string, std::shared_ptr<nodo>>* get_nodos() const { return nullptr; }
        virtual const std::vector<option_base*>* get_options() const { return nullptr; };
    };
	class cmd : public nodo {
		func_cmd func;
        const std::vector<option_base*> options;
	public:
        cmd(const func_cmd& func, std::vector<option_base*>&& options = {}, const std::string& manual = "")
			:nodo(manual), options(std::move(options)), func(func) {}
        ~cmd()
        {
            for (auto op : options)
                delete op;
        }
		int operator()(const std::string& rotulo, terminal& raiz) const override;
        const std::vector<option_base*>* get_options() const override { return &options; };

	};
public:
	class sub : public nodo {
		friend terminal;
	private:
		std::map<std::string, std::shared_ptr<nodo>> nodos;
	public:
		sub(const std::string& manual = "") :nodo(manual) {}
		sub& novo(std::string identificador, const func_cmd& func);
		sub& novo(std::string identificador, const std::string& manual, const func_cmd& func);
        sub& novo(std::string identificador, std::vector<option_base*>&& options, const func_cmd& func);
        sub& novo(std::string identificador, const sub& novo_sub);
		int operator()(const std::string& rotulo, terminal& raiz) const override;
		const std::map<std::string, std::shared_ptr<nodo>>* get_nodos() const override { return &nodos; }

	};
private:
	const sub raiz;
	std::stringstream entrada;
	int auto_voltar = 0;
public:
	terminal(const sub& raiz) :raiz(raiz) {};
	void executar();
private:
	int executar(const std::string& rotulo, const nodo& n);
};