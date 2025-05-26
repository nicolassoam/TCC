#ifndef GP_H_
#define GP_H_
#include <iostream>
#include <vector>
#include "util.h"

#define POPULATION_SIZE 40
#define TOURNAMENT_SIZE 3
#define AIRCRAFT_TYPES 7
#define CONSTRAINT_PEN -1000000
#define BIG_M 20000
#define MAX_ASSIGNED_MODELS 300
#define MAX_ASSIGNED_TYPES 7

using IntMatrix = std::vector<std::vector<int>>;
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
    
    Chromossome (int totalAircraftsTypes, int flightLegs, int timeWindows)
    {
        flightData.resize(flightLegs);
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
    Population initializePopulation(int populationSize, int aircraftTypes, int flightLegs, int timeWindows);
    Population newGen(Population& population, InstanceType instance,float cr, float mr);
    void evaluateIndividual(Individual& ind, InstanceType instance, 
                            StringMatrix flightLegPrices, StringMatrix caskValues, InstanceType flightsMap);
    int constraintCheck(Individual& ind, InstanceType instance, InstanceType flightsMap);
    int tournament(Population population, int k);
    void crossover(Individual& fstMate, Individual& sndMate);
    void mutate(Individual& mutated);
    Individual search(InstanceType flightLegs, StringMatrix cask, StringMatrix passagem, 
                      InstanceType instance, int generations, 
                      int populationSize, float mr = 0.1, float cr = 0.9);
}

#endif