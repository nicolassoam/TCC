#ifndef GP_H_
#define GP_H_
#include <iostream>
#include <vector>
#include "util.h"

#define POPULATION_SIZE 300
#define TOURNAMENT_SIZE 5
#define AIRCRAFT_TYPES 7
#define CONSTRAINT_PEN -15
#define BIG_M 20000
#define MAX_ASSIGNED_MODELS 300
#define MAX_ASSIGNED_TYPES 7

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
    int flightFrequency;
    int passengerNumber;

    Flight(int flightFrequency, int passengerNumber)
    {
        this->flightFrequency = flightFrequency;
        this->passengerNumber = passengerNumber;
    };
    Flight () {};
    ~Flight () {};
};
using FlightMatrix = std::vector<std::vector<Flight>>;
struct Chromossome
{
    std::vector<FlightMatrix> flightData;
    std::vector<Aircraft> allowedAircraft;
    Chromossome (int totalAircraftsTypes, int flightLegs, int timeWindows)
    {
        flightData.resize(flightLegs);
        allowedAircraft.resize(totalAircraftsTypes);
        for(int i = 0; i < flightLegs; i++)
        {
            flightData[i].resize(timeWindows);
            for(int j = 0; j < timeWindows; j++)
            {
                flightData[i][j].resize(totalAircraftsTypes, Flight(0, 0));
            }
        }
    };
    Chromossome () {}; 
    ~Chromossome () {};

};

struct Individual
{
   float fitness;
   // TODO: Chromossome representation 
   Chromossome ch;
   Individual (int totalAircraftsTypes, int flightLegs, int timeWindows) 
   {
    fitness = 0;
    ch = Chromossome(totalAircraftsTypes, flightLegs, timeWindows);
   };
   ~Individual () {};   
};

using Population = std::vector<Individual>;
namespace GP
{
    Population initializePopulation(InstanceType flightsMap,StringMatrix aircrafts, int populationSize, int aircraftTypes, int flightLegs, int timeWindows);
    Population newGen(Population& population, InstanceType flightsMap, InstanceType instance, const StringMatrix& passagem, const StringMatrix& cask ,float cr, float mr);
    void evaluateIndividual(Individual& ind, InstanceType instance, 
                            StringMatrix flightLegPrices, StringMatrix caskValues, InstanceType flightsMap);
    long constraintCheck(Individual& ind, InstanceType instance, InstanceType flightsMap);
    int tournament(Population population, int k);
    std::pair<Individual, Individual> crossover(const Individual& fstMate, const Individual& sndMate, const StringMatrix& aircrafts, const InstanceType& flightsMap);
    void mutate(Individual& ind, const InstanceType& instance, const StringMatrix& aircrafts, const InstanceType& flightsMap, const StringMatrix& passagem, const StringMatrix& cask);
    void mutateHillClimb(Individual& ind, const InstanceType& instance, const StringMatrix& aircrafts, const InstanceType& flightsMap, const StringMatrix& passagem, const StringMatrix& cask);
    void mutateAddFlight(Individual& ind, const InstanceType& instance, const StringMatrix& aircrafts, const InstanceType& flightsMap);
    void mutateRemoveFlight(Individual& ind);
    void mutateSwapAircraft(Individual& ind, const StringMatrix& aircrafts);
    void mutateAdjustPassengers(Individual& ind, const InstanceType& instance, const StringMatrix& aircrafts, const InstanceType& flightsMap);
    void mutateFlipAircraftType(Individual& ind, const StringMatrix& aircrafts, const InstanceType& flightsMap);
    void fixFlippedAircraft(Individual& ind, int flippedAircraft, const StringMatrix& aircrafts, const InstanceType& flightsMap);
    void repairIndividual(Individual& ind);
    void adequateAircraftAmount(Individual& ind);
    void repairReturnFlightFrequencies(Individual& ind, int legIndex, int timeWindow);
    Individual search(InstanceType flightLegs, StringMatrix cask, StringMatrix passagem, 
                      InstanceType instance, int generations, 
                      int populationSize, float mr = 0.4, float cr = 0.8);
}

namespace util
{
    void writeBestIndividual(Individual& ind, InstanceType flightLegs, InstanceType instance);
    void sortBestFlight(Individual& best);
}
#endif