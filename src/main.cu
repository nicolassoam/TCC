#include "util.h"
#include "gp.h"
#include <iostream>
#include <vector>
#include <string>
#include "gpu.cuh"
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
    std::cout << "Origin :" << routes[0].originIcao << std::endl;
    std::cout << "Destination :" << routes[0].destinationIcao << std::endl;
    std::cout << "Demand in TER1 :" << routes[0].demandPerWindow[4] << std::endl;
    std::cout << "Origin :" << routes[10].originIcao << std::endl;
    std::cout << "Destination :" << routes[10].destinationIcao << std::endl;
    std::cout << "Demand in TER3 :" << routes[10].demandPerWindow[6]<< std::endl;

    std::cout << "Flight legs size: " << flightLegs.size() << std::endl;  
    std::vector<Individual> bestIndividuals;
    std::vector<double> timesToFinish;
    std::chrono::duration<double> elapsed;
    int generations = 7000;
#ifdef GPU_ON
    std::vector<double> elapsedTransferTimes;
    GPU::DeviceDataManager deviceData;
    GPU::setupDeviceData(deviceData, aircraftTypes, routes, TIME_WINDOWS);
#endif
    for (int i = 0; i < 3; i++)
    {
        hClock startTime = std::chrono::high_resolution_clock::now();
#ifndef GPU_ON
        Individual individual = GP::search(routes, aircraftTypes, generations, POPULATION_SIZE);
#else
        Individual individual = GP::search(routes, aircraftTypes, generations, POPULATION_SIZE, deviceData, elapsedTransferTimes);
#endif
        hClock endTime = std::chrono::high_resolution_clock::now();
        elapsed = endTime - startTime;
        bestIndividuals.push_back(individual);
        timesToFinish.push_back(elapsed.count());
    }
    std::cout << elapsed.count() << std::endl;
    std::sort(bestIndividuals.begin(), bestIndividuals.end(), [](Individual& a, Individual& b) {return a.fitness > b.fitness;});

    util::writeBestIndividual(bestIndividuals[0], flightLegs, instances, routes, aircraftTypes);
#ifdef GPU_ON
    util::writeSolutionTimes(timesToFinish, elapsedTransferTimes);
#else
    util::writeSolutionTimes(timesToFinish);
#endif
    return 0;
}