#include <iostream>
#include "terminal.h"


int main() 
{
    typedef terminal::sub sub;
    typedef std::stringstream ss;
    terminal raiz(sub()
        .novo("ctrl", sub()
            .novo("config", sub()
                .novo("min",
                    {
                        (option_base*)new option<float>({"--value","-v"}, 0.0f, "value to which to set the minimum"),
                        (option_base*)new option<bool>({"--sens"}, false, "set the minimum to the current sensor reading")
                    },
                    [](const cmd_args& in) {
                        auto& sens = *(option<bool>*)in[1];
                        if (sens.val)
                        {
                            std::cout << "min set to current sensor reading\n";
                        }
                        else
                        {
                            auto& value = *(option<float>*)in[0];
                            std::cout << "min set manually to " << value.val << std::endl;
                        }
                    }
                )
                .novo("max",
                    {
                        (option_base*)new option<float>({"--value","-v"}, 0.0f, "value to which to set the maximum"),
                        (option_base*)new option<bool>({"--sens"}, false, "set the maximum to the current sensor reading")
                    },
                    [](const cmd_args& in) {
                        auto& sens = *(option<bool>*)in[1];
                        if (sens.val)
                        {
                            std::cout << "max set to current sensor reading\n";
                        }
                        else
                        {
                            auto& value = *(option<float>*)in[0];
                            std::cout << "max set manually to " << value.val << std::endl;
                        }
                    }
                )
                .novo("pausa",
                    [](const cmd_args& in) {
                        static bool pausa = true;
                        pausa = !pausa;
                        std::cout << "Controlador " << (pausa ? "pausado\n" : "ligado\n");
                    }
                )
            )
        )
        .novo("plot",
            [](const cmd_args& in) {
                std::cout << "plotando...\n";
            }
        )
    );

    raiz.executar();
	return 0;
}