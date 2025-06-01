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
    StringMatrix passagem = util::passagem(instances);
    std::vector<Flight> flights;
    StringMatrix prices = util::mapDestinationToAircraftTicketPrice(passagem, flightData);
    StringMatrix caskValues = util::mapDestinationToAircraftTicketPrice(cask, flightData);
    InstanceType flightLegs = util::FlightLegs(flightData, passagem);
     
    std::cout << "Flight legs size: " << flightLegs.size() << std::endl;    
    // Individual individual = GP::search(flightLegs, caskValues, prices, instances, 500, POPULATION_SIZE);
    // std::cout << "Best fitness: " << individual.fitness << std::endl;
    return 0;
}