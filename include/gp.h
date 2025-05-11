#ifndef GP_H_
#define GP_H_
#include <iostream>
#include <vector>
#include "util.h"
#include "enumTypedef.h"

#define POPULATION_SIZE 100
#define TOURNAMENT_SIZE 5

// using Chromossome = std::vector<std::vector<int>>;
using IntMatrix = std::vector<std::vector<int>>;
struct Chromossome
{
    std::vector<int> fleetSize;
    std::vector<int> aircraftTypeDecisionVariable1;
    IntMatrix aircraftTypeDecisionVariable2;
    std::vector<IntMatrix> flightFrequency;
    std::vector<IntMatrix> passengerNumber;
    Chromossome (int totalAircraftsTypes, int flightLegs, int timeWindows)
    {
        fleetSize.reserve(totalAircraftsTypes);
        aircraftTypeDecisionVariable1.reserve(totalAircraftsTypes);
        
        aircraftTypeDecisionVariable2.reserve(flightLegs);
        aircraftTypeDecisionVariable2.assign(flightLegs, std::vector<int>(totalAircraftsTypes));

        flightFrequency.reserve(flightLegs);
        flightFrequency.assign(flightLegs, IntMatrix(timeWindows, std::vector<int>(totalAircraftsTypes)));

        passengerNumber.reserve(flightLegs);
        passengerNumber.assign(flightLegs, IntMatrix(timeWindows, std::vector<int>(totalAircraftsTypes)));
    } 
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
    Population newGen(Population& population,float cr, float mr);
    void evaluateIndividual(Individual& ind, InstanceType instance);
    int tournament(Population population, int k);
    void crossover(Individual& fstMate, Individual& sndMate);
    void mutate(Individual& mutated);
    Individual search(int generations, int populationSize, float mr = 0.1, float cr = 0.9);
}

#endif