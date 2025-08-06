#ifndef GP_H_
#define GP_H_
#include <iostream>
#include <vector>
#include <random>
#include "util.h"
#include "gpu.cuh"

using IntMatrix = std::vector<std::vector<int>>;

namespace GP
{
    Population initializePopulation(const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes);
    void evaluateIndividual(Individual& ind, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes);
    int tournament(const Population &population, int k);
    std::pair<Individual, Individual> crossover(const Individual& parent1, const Individual& parent2, const std::vector<AircraftType>& aircraftTypes, const std::vector<Route>& routes);
    void mutate(Individual& individual, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes);
    void mutateAddRoute(Individual& individual, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes);
    void mutateAdjustPassengers(Individual& individual, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes);
    void repair(Individual& child, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes);
    Individual search(const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes, int generations,
        int populationSize, GPU::DeviceDataManager deviceData = GPU::DeviceDataManager());
    void getServedDemand(const Individual& individual, std::vector<int>& servedDemand);
    std::map<int, int> getTotalFleetRequired(const Individual& individual);
    int calculateRequiredFleetSize(const Individual& individual, int aircraftTypeId);
    void cleanupSchedule(Individual& individual);
}

namespace util
{
    void writeBestIndividual(Individual& ind, InstanceType flightLegs, InstanceType instance, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftType);
    void writeSolutionTimes(std::vector<double>elapsedTimes);
    void writeFlightsPerDestination(const Individual& bestIndividual, const std::vector<Route>& routes);
}
#endif