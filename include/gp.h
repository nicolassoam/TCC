#ifndef GP_H_
#define GP_H_
#include <iostream>
#include <vector>
#include "util.h"

#define POPULATION_SIZE 40
#define TOURNAMENT_SIZE 3
#define AIRCRAFT_TYPES 7
#define CONSTRAINT_PEN -1000000000
#define BIG_M 20000
#define MAX_ASSIGNED_MODELS 300
#define MAX_ASSIGNED_TYPES 7

using IntMatrix = std::vector<std::vector<int>>;

using PassengerMatrix = std::vector<std::vector<int>>;
struct Aircraft
{
    int index = 0;
    int count = 0;
    PassengerMatrix passengerPerFlight;
    Aircraft(int index, int flightLegs, int timeWindows)
    {
        this->index = index;
        passengerPerFlight.resize(flightLegs);
        for(int i = 0; i < flightLegs; i++)
        {
            passengerPerFlight[i].resize(timeWindows, 0);
        }
    };
};


struct Chromossome
{
    std::vector<Aircraft> aircraft;
    
    Chromossome (int totalAircraftsTypes, int flightLegs, int timeWindows)
    {
        aircraft.resize(totalAircraftsTypes);
        for(int i = 0; i < totalAircraftsTypes; i++)
        {
            aircraft[i] = Aircraft(i, flightLegs, timeWindows);
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
    Population initializePopulation(int populationSize, int aircraftTypes, int flightLegs, int timeWindows);
    Population newGen(Population& population, InstanceType instance,float cr, float mr);
    void evaluateIndividual(Individual& ind, InstanceType instance, 
                            StringMatrix flightLegPrices, StringMatrix caskValues, InstanceType flightsMap);
    long constraintCheck(Individual& ind, InstanceType instance, InstanceType flightsMap);
    int tournament(Population population, int k);
    void crossover(Individual& fstMate, Individual& sndMate);
    void mutate(Individual& mutated);
    Individual search(InstanceType flightLegs, StringMatrix cask, StringMatrix passagem, 
                      InstanceType instance, int generations, 
                      int populationSize, float mr = 0.9, float cr = 0.9);
}

namespace util
{
    void writeBestIndividual(Individual& ind, InstanceType instance);
}
#endif