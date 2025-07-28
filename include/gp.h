#ifndef GP_H_
#define GP_H_
#include <iostream>
#include <vector>
#include "util.h"
#include <random>
#define POPULATION_SIZE 500
#define TOURNAMENT_SIZE 5
#define AIRCRAFT_TYPES 7
#define CONSTRAINT_PEN -15
#define BIG_M 20000
#define MAX_ASSIGNED_MODELS 300
#define MAX_ASSIGNED_TYPES 2
#define TIME_WINDOWS 28
#define HUB 0

using IntMatrix = std::vector<std::vector<int>>;

struct Aircraft
{
    bool allowed = false;
    int count = 0;
    Aircraft() {};
    ~Aircraft() {};
};


struct Flight
{
    int aircraft_type_id;
    int outbound_route_id; 

    int outbound_window;
    int return_window;     

    int frequency;
    int outbound_passengers_per_flight;
    int return_passengers_per_flight;
};
using FlightMatrix = std::vector<std::vector<Flight>>;
using FlightVector = std::vector<Flight>;
struct Individual
{
   float fitness;
   std::vector<Flight> schedule;
   std::vector<bool> allowedAircraft;
   Individual () 
   {
    fitness = 0;
    allowedAircraft.resize(AIRCRAFT_TYPES);
   };
   ~Individual () {};   
};

using Population = std::vector<Individual>;
namespace GP
{
    Population initializePopulation(std::vector<Route> routes, std::vector<AircraftType> aircraftTypes);
    //bool isFlowValid(const Individual& individual, std::vector<Route> routes);
    void evaluateIndividual(Individual& ind, std::vector<Route> routes, std::vector<AircraftType> aircraftTypes);
    int tournament(Population population, int k);
    std::pair<Individual, Individual> crossover(const Individual& parent1, const Individual& parent2, std::vector<AircraftType> aircraftTypes, std::vector<Route> routes);
    void mutate(Individual& individual, std::vector<Route> routes, std::vector<AircraftType> aircraftTypes);
    void repair(Individual& child, std::vector<Route> routes, std::vector<AircraftType> aircraftTypes);
    Individual search(std::vector<Route> routes, std::vector<AircraftType> aircraftTypes, int generations,
                      int populationSize, float mr = 0.4, float cr = 0.65);


}

namespace util
{
    void writeBestIndividual(Individual& ind, InstanceType flightLegs, InstanceType instance, std::vector<Route> routes, std::vector<AircraftType> aircraftType);
    //void sortBestFlight(Individual& best);
}
#endif