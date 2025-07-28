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
    std::vector<Route> routes;
    std::vector<AircraftType> aircraftTypes;
    StringMatrix prices = util::mapDestinationToAircraftTicketPrice(passagem, flightData);
    StringMatrix caskValues = util::mapDestinationToAircraftTicketPrice(cask, flightData);
    StringMatrix aircrafts = util::aeronave(instances);
    InstanceType flightLegs = util::FlightLegs(flightData, passagem);
    routes = util::readRoute(prices, caskValues, flightLegs);
    aircraftTypes = util::readAicrafts(aircrafts);
    std::cout << "Origin :" << routes[0].origin_icao << std::endl;
    std::cout << "Destination :" << routes[0].destination_icao << std::endl;
    std::cout << "Origin :" << routes[10].origin_icao << std::endl;
    std::cout << "Destination :" << routes[10].destination_icao << std::endl;

    for (auto a : aircraftTypes)
    {
        std::cout << "Id: " << a.id
            << " Name: " << a.name
            << " Capacity: " << a.capacity
            << " Range: " << a.range_km << std::endl;
    }
    std::cout << "Flight legs size: " << flightLegs.size() << std::endl;  
    std::vector<Individual> bestIndividuals;
    for (int i = 0; i < 1; i++)
    {
        Individual individual = GP::search(routes, aircraftTypes, 400, POPULATION_SIZE);
        bestIndividuals.push_back(individual);
    }
    
    std::sort(bestIndividuals.begin(), bestIndividuals.end(), [](Individual& a, Individual& b) {return a.fitness > b.fitness;});
    util::writeBestIndividual(bestIndividuals[0], flightLegs, instances, routes, aircraftTypes);
    return 0;
}