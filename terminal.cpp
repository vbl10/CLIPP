#include "terminal.h"
#include <iostream>
#include <iomanip>

std::istream& operator>>(std::istream& in, option_base& op)
{
    op.parse(in);
    return in;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const option<T>& op)
{
    out << op.val;
    return out;
}
void parse_options(const std::vector<option_base*>& options, std::istream& in)
{
    std::string id;
    while (in.good())
    {
        in >> id;
        auto it = std::find_if(options.begin(), options.end(),
            [&id](option_base* option) {
                return option->aliases.end() !=
                    std::find(option->aliases.begin(), option->aliases.end(), id);
            }
        );
        if (it != options.end())
        {
            (*it)->parse(in);
        }
    }
}


int terminal::cmd::operator()(const std::string& rotulo, terminal& raiz) const
{
    parse_options(options, raiz.entrada);
    func(options);
    return 0;
}

terminal::sub& terminal::sub::novo(std::string identificador, const func_cmd& novo_cmd)
{
    nodos[identificador] = std::make_shared<cmd>(novo_cmd);
    return *this;
}

terminal::sub& terminal::sub::novo(std::string identificador, const std::string& manual, const func_cmd& novo_cmd)
{
    nodos[identificador] = std::make_shared<cmd>(novo_cmd, std::vector<option_base*>{}, manual);
    return *this;
}

terminal::sub& terminal::sub::novo(std::string identificador, std::vector<option_base*>&& options, const func_cmd& func)
{
    nodos[identificador] = std::make_shared<cmd>(func, std::move(options));
    return *this;
}

terminal::sub& terminal::sub::novo(std::string identificador, const sub& novo_sub)
{
    nodos[identificador] = std::make_shared<sub>(novo_sub);
    return *this;
}

int terminal::sub::operator()(const std::string& rotulo, terminal& raiz) const
{
    return raiz.executar(rotulo, *this);
}

void terminal::executar()
{
    while (executar("", raiz) != -2);
}

int terminal::executar(const std::string& rotulo, const nodo& n)
{
    const std::map<std::string, std::shared_ptr<nodo>>& comandos = *n.get_nodos();
    while (true) {
        //ler nova entrada
        if (entrada.str().empty() || !entrada.good()) {
            std::cout << rotulo << (rotulo.empty() ? "> " : " ");
            char buffer[256];
            std::cin.getline(buffer, 255);
            entrada.clear();
            entrada.str(buffer);
            auto_voltar = 0;
        }
        else {
            auto_voltar++;
        }

        std::string identificador;
        entrada >> identificador;

        auto it = comandos.find(identificador);

        if (it != comandos.end()) {
            int r = (*(it->second))(rotulo + identificador + '>', *this);
            
            entrada.str("");

            if (r >= 0) {
                r += auto_voltar;
                auto_voltar = 0;
            }

            if (r) {
                return r > 0 ? r - 1 : r;
            }
        }
        else if (identificador == "ajuda") {

            //imprimir todos os comandos
            for (const auto& cmd : comandos)
                printf("%s\n", cmd.first.c_str());

            std::cout <<
                "ajuda\n"
                "desc\n"
                "voltar\n"
                "inicio\n"
                "sair\n"
                "sys\n";
            entrada.str("");
            if (auto_voltar)
                return auto_voltar - 1;
        }
        else if (identificador == "manual") {
            //encontrar alvo
            const std::map<std::string, std::shared_ptr<nodo>>* nodos = &comandos;
            const nodo* alvo = &n;
            while (entrada.good())
            {
                entrada >> identificador;
                it = nodos->find(identificador);
                if (it != nodos->end()) {
                    alvo = it->second.get();
                    nodos = alvo->get_nodos();
                    if (!nodos)
                        break;
                }
            }
            //imprimir manual
            std::cout << alvo->desc << std::endl;
            std::cout << "Uso: " << identificador;
            if (alvo->get_options() && alvo->get_options()->size())
            {
                std::cout << " [opcoes]\nOpcoes:\n";
                
                for (auto op : *alvo->get_options())
                {
                    std::string aliases(2, ' ');
                    for (int i = 0; i < op->aliases.size(); i++)
                    {
                        aliases += op->aliases[i];
                        if (i != op->aliases.size() - 1)
                        {
                            aliases += ", ";
                        }
                    }
                    const char* type = op->get_typename();
                    if (strlen(type))
                    {
                        aliases += " [";
                        aliases += type;
                        aliases += "] ";
                    }
                    std::cout
                        << std::setfill(' ')
                        << std::setw(25)
                        << std::setiosflags(std::ios_base::left);
                    std::cout << aliases;
                    std::cout << op->desc << std::endl;
                }
                std::cout << std::endl;
                std::cout << std::setw(0);
            }
            else std::cout << std::endl;
            std::cout << std::endl;
            entrada.str("");
        }
        else if (identificador == "voltar") {
            entrada.str("");
            return 0;
        }
        else if (identificador == "inicio") {
            entrada.str("");
            return -1;
        }
        else if (identificador == "sair") {
            entrada.str("");
            return -2;
        }
        else if (identificador == "sys") {
            char cmd[256];
            entrada.getline(cmd, 255);
            system(cmd);
            entrada.str("");
        }
        else if (!identificador.empty()) {
            entrada.str("");
            printf("Comando \"%s\" desconhecido.\n\n", identificador.c_str());
        }
    }
    return 0;
}