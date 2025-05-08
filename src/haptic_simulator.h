#pragma once
#include <iostream>

class HapticSimulator {  // Alterado para class ao invés de struct
public:
    static void init() {
        std::cout << "Simulação háptica inicializada\n";
    }
    
    static void shutdown() {
        std::cout << "Simulação háptica finalizada\n";
    }
};
