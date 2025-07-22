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
    srand(10);
    std::cout << "Flight legs size: " << flightLegs.size() << std::endl;  
    std::vector<Individual> bestIndividuals;
    for (int i = 0; i < 1; i++)
    {
        Individual individual = GP::search(flightLegs, caskValues, prices, instances, 300, POPULATION_SIZE);
        bestIndividuals.push_back(individual);
    }
    std::sort(bestIndividuals.begin(), bestIndividuals.end(), [](Individual& a, Individual& b) {return a.fitness > b.fitness;});
    util::writeBestIndividual(bestIndividuals[0], flightLegs, instances);
    return 0;
}