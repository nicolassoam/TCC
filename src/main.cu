#include "util.h"
#include <iostream>
#include <vector>
#include <string>

int main()
{
    
    InstanceType instances = util::returnInstance();
    StringMatrix cask = util::cask();
    /*for (const auto& instance : instances) {
        for (const auto& line : instance) {
            std::cout << line << std::endl;
        }
        std::cout << "-----------------" << std::endl;
    }*/

    /*for (const auto& line : instances[CASK])
    {
        std::cout << line << " ";
    }*/

    std::cout << instances[CASK][CaskPassagem::E190_E2][1] << " ";
    return 0;
}