#include "util.h"
#include "gp.h"
#include <iostream>
#include <vector>
#include <string>

int main()
{
     
    InstanceType instances = util::loadInstance();
    StringMatrix cask = util::cask(instances);
    StringMatrix flightData = util::rotas2(instances);
    std::vector<Flight> flights;

    Individual individual = GP::search(instances,500, POPULATION_SIZE);
    std::cout << "Best fitness: " << individual.fitness << std::endl;
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

    std::cout << util::rotas2(instances)[0][1] << " ";
    return 0;
}