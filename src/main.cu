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
    Individual individual = GP::search(flightLegs, caskValues, prices, instances, 50, POPULATION_SIZE);
    util::writeBestIndividual(individual);
    return 0;
}