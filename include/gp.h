#ifndef GP_H_
#define GP_H_
#include <iostream>
#include <vector>
#include <random>
#include "util.h"
#include "gpu.cuh"
#define CONSTRAINT_PEN -15
#define BIG_M 20000
#define MAX_ASSIGNED_MODELS 300


using IntMatrix = std::vector<std::vector<int>>;

namespace GP
{
    Population initializePopulation(std::vector<Route> routes, std::vector<AircraftType> aircraftTypes);
    void evaluateIndividual(Individual& ind, std::vector<Route> routes, std::vector<AircraftType> aircraftTypes);
    int tournament(Population population, int k);
    std::pair<Individual, Individual> crossover(const Individual& parent1, const Individual& parent2, std::vector<AircraftType> aircraftTypes, std::vector<Route> routes);
    void mutate(Individual& individual, std::vector<Route> routes, std::vector<AircraftType> aircraftTypes);
    void mutateAddRoute(Individual& individual, std::vector<Route> routes, std::vector<AircraftType> aircraftTypes);
    void mutateAdjustPassengers(Individual& individual, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes);
    void repair(Individual& child, std::vector<Route> routes, std::vector<AircraftType> aircraftTypes);
    Individual search(std::vector<Route> routes, std::vector<AircraftType> aircraftTypes, int generations,
        int populationSize, GPU::DeviceDataManager deviceData = GPU::DeviceDataManager());
    std::map<std::pair<int, int>, int> getServedDemand(const Individual& individual);
    std::map<int, int> getTotalFleetRequired(const Individual& individual);
    int calculateRequiredFleetSize(const Individual& individual, int aircraftTypeId);
}

namespace util
{
    void writeBestIndividual(Individual& ind, InstanceType flightLegs, InstanceType instance, std::vector<Route> routes, std::vector<AircraftType> aircraftType);
    void writeSolutionTimes(std::vector<double>elapsedTimes);
}
#endif