#include <iostream>
#include "Simulator.h"

int main() {
    std::cout << "CrossLink-FG Frame Generation Middleware Simulator Starting..." << std::endl;

    try {
        Simulator simulator;
        simulator.Initialize();
        simulator.Run();
        simulator.Shutdown();

        std::cout << "Simulation completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
