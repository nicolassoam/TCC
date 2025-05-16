#ifndef GP_H_
#define GP_H_
#include <iostream>
#include <vector>
#include "util.h"

#define POPULATION_SIZE 100
#define TOURNAMENT_SIZE 5
#define AIRCRAFT_TYPES 7
#define CONSTRAINT_PEN -10000
#define BIG_M 20000
#define MAX_ASSIGNED_MODELS 300
#define MAX_ASSIGNED_TYPES 7
#define MAX_TRIES 2

using IntMatrix = std::vector<std::vector<int>>;
struct Flight
{
    String od;
    String origin;
    String destination;
    int runway;
    String timeWindow;
    int demand;
    double distance;
    Flight(String od, String origin, String destination, int runway, String timeWindow, int demand, int distance)
    {
        this->od = od;
        this->origin = origin;
        this->destination = destination;
        this->runway = runway;
        this->timeWindow = timeWindow;
        this->demand = demand;
        this->distance = distance;
    };
    Flight () {};
    ~Flight () {};
};
struct Chromossome
{
    std::vector<int> fleetSize;
    std::vector<int> aircraftTypeDecisionVariable1;
    IntMatrix aircraftTypeDecisionVariable2;
    std::vector<IntMatrix> flightFrequency;
    std::vector<IntMatrix> passengerNumber;
    
    Chromossome (int totalAircraftsTypes, int flightLegs, int timeWindows)
    {
        this->fleetSize.reserve(totalAircraftsTypes);
        this->aircraftTypeDecisionVariable1.reserve(totalAircraftsTypes);
        
        this->aircraftTypeDecisionVariable2.reserve(flightLegs);
        this->aircraftTypeDecisionVariable2.assign(flightLegs, std::vector<int>(totalAircraftsTypes));

        this->flightFrequency.reserve(flightLegs);
        this->flightFrequency.assign(flightLegs, IntMatrix(timeWindows, std::vector<int>(totalAircraftsTypes)));

        this->passengerNumber.reserve(flightLegs);
        this->passengerNumber.assign(flightLegs, IntMatrix(timeWindows, std::vector<int>(totalAircraftsTypes)));
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
                            std::map<String,std::vector<double>> flightLegPrices, std::map<String, std::vector<double>> caskValues);
    int constraintCheck(Individual& ind, InstanceType instance);
    int tournament(Population population, int k);
    void crossover(Individual& fstMate, Individual& sndMate);
    void mutate(Individual& mutated);
    Individual search(std::map<String,std::vector<double>> flightLegPrices, std::map<String, std::vector<double>> caskValues, 
                      InstanceType instance, int generations, 
                      int populationSize, float mr = 0.1, float cr = 0.9);
}

#endif