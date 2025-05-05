#ifndef GP_H_
#define GP_H_
#include <iostream>
#include <vector>

#define POPULATION_SIZE 100
#define TOURNAMENT_SIZE 5

struct Individual
{
   float fitness;
   // TODO: Chromossome representation
};
using Population = std::vector<Individual>;
namespace GP
{
    Population initializePopulation(int populationSize);
    Population newGen(Population& population,float cr, float mr);
    void evaluatePopulation();
    int tournament(Population population, int k);
    void crossover(Individual& fstMate, Individual& sndMate);
    void mutate(Individual& mutated);
    Individual search(int generations, int populationSize, float mr = 0.1, float cr = 0.9);
}

#endif