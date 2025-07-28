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
    for (auto r : routes)
    {
        std::cout << "id: " << r.id
            << " Origin: " << r.origin_icao
            << " Destination: " << r.destination_icao
            << " Distance: " << r.distance_km << std::endl
            << " Demands: " << std::endl;
        for (auto demand : r.demand_per_window)
        {
            std::cout << demand << std::endl;
        }
        std::cout << " Prices: " << std::endl;
        for (auto i : r.ticket_prices) 
        {
            std::cout << i.first << ", " << i.second << std::endl;
        }

    }

    for (auto a : aircraftTypes)
    {
        std::cout << "Id: " << a.id
            << " Name: " << a.name
            << " Capacity: " << a.capacity
            << " Range: " << a.range_km << std::endl;
    }
   /* std::cout << "Flight legs size: " << flightLegs.size() << std::endl;  
    std::vector<Individual> bestIndividuals;
    for (int i = 0; i < 1; i++)
    {
        Individual individual = GP::search(flightLegs, caskValues, prices, instances, 2000, POPULATION_SIZE);
        bestIndividuals.push_back(individual);
    }
    std::sort(bestIndividuals.begin(), bestIndividuals.end(), [](Individual& a, Individual& b) {return a.fitness > b.fitness;});
    util::writeBestIndividual(bestIndividuals[0], flightLegs, instances);*/
    return 0;
}