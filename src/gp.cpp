#include "gp.h"
#include <algorithm>
#include <execution>

#define AIRCRAFT_TYPES 7
#define CONSTRAINT_PEN 1000000
namespace GP
{

    int tournament(Population population, int k){
        int best_index = rand() % POPULATION_SIZE;
        Individual* best = &population[best_index];
        for(int i = 0; i < k; i++){
            int random_index = rand() % POPULATION_SIZE;
            Individual* random = &population[random_index];
            if(random->fitness < best->fitness){
                best = random;
                best_index = random_index;
            }
        }
        return best_index;
    }

    Population newGen(Population& population,float cr, float mr)
    {
        Population children;

        short firstParent = 0;
        short secondParent = 0;

        for(int i = 0; i < POPULATION_SIZE; i+=2)
        {
            do
            {
                firstParent = tournament(population, TOURNAMENT_SIZE);
                secondParent = tournament(population, TOURNAMENT_SIZE);
            } while (firstParent == secondParent);

            Individual fstChild = population[firstParent];
            Individual sndChild = population[secondParent];
            
            if(rand()%100 < cr * 100)
            {
                crossover(fstChild, sndChild);
            }
            if(rand()%100 < mr * 100)
            {
                mutate(fstChild);
                mutate(sndChild);
            }
            children.push_back(fstChild);
            children.push_back(sndChild);
        }
        return children;
    }

    Population initializePopulation(int populationSize, int aircraftTypes, int flightLegs, int timeWindows)
    {
        Population pop;

        for(int i = 0; i < populationSize; i++)
        {
            Individual ind = Individual(aircraftTypes, flightLegs, timeWindows);

            std::vector<int>& fleetSize = ind.ch.fleetSize;
            std::vector<int>& aircraftTypeDecisionVariable1 = ind.ch.aircraftTypeDecisionVariable1;
            IntMatrix& aircraftTypeDecisionVariable2 = ind.ch.aircraftTypeDecisionVariable2;
            std::vector<IntMatrix>& flightFrequency = ind.ch.flightFrequency;
            std::vector<IntMatrix>& passengerNumber = ind.ch.passengerNumber;
            for(int j = 0; j < aircraftTypes; j++)
            {
                fleetSize.push_back(rand() % 300 + 1);
                aircraftTypeDecisionVariable1.push_back(rand() % 1);
            }

            for(int k = 0; k < flightLegs; k++)
            {
                for(int l = 0; l < aircraftTypes; l++)
                {
                    aircraftTypeDecisionVariable2[k][l] = rand() % 1;
                }

                for(int m = 0; m < timeWindows; m++)
                {
                    for(int n = 0; n < aircraftTypes; n++)
                    {
                        flightFrequency[k][m][n] = rand() % 300;
                        passengerNumber[k][m][n] = rand() % 300;
                    }
                }

            }
            pop.push_back(ind);
        }

        return pop;
    }

    void evaluateIndividual (Individual& ind, InstanceType instance)
    {
        // pick vectors from individual
        std::vector<int>& fleetSize = ind.ch.fleetSize;
        std::vector<int>& aircraftTypeDecisionVariable1 = ind.ch.aircraftTypeDecisionVariable1;
        IntMatrix& aircraftTypeDecisionVariable2 = ind.ch.aircraftTypeDecisionVariable2;
        std::vector<IntMatrix>& flightFrequency = ind.ch.flightFrequency;
        std::vector<IntMatrix>& passengerNumber = ind.ch.passengerNumber;

        //check constraints, if violated worst fitness
    }

    Individual search(InstanceType instance, int generations, int populationSize, float mr, float cr)
    {
        Population pop;
        int aircraftTypes = util::passagem().size() - 2;
        int flightLegs = util::cask()[CaskPassagem::DESTINO].size();
        int timeWindows = util::rotas2()[Rotas2_3::TURNO].size();
        
        pop = initializePopulation(populationSize, aircraftTypes, flightLegs, timeWindows);

        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            evaluateIndividual(pop[i]);
        }

        std::sort(std::execution::par_unseq,pop.begin(), pop.end(), [](Individual& a, Individual& b){return a.fitness < b.fitness;});
        Individual best = pop[0];

        std::cout << "Best fitness: " << best.fitness << std::endl;

        for (int i = 0; i < generations; i++)
        {
            Population children = newGen(pop, mr, cr);
            // worst fitness first
            std::sort(std::execution::par_unseq,pop.begin(), pop.end(), [](Individual& a, Individual& b){return a.fitness > b.fitness;});
            pop[0] = best;
            std::sort(std::execution::par_unseq,pop.begin(), pop.end(), [](Individual& a, Individual& b){return a.fitness < b.fitness;});

            if(pop[0].fitness < best.fitness)
            {
                best = pop[0];
            }
            std::cout << "Generation: " << i << " Best fitness: " << best.fitness << std::endl;
        }

        std::cout << "Best fitness: " << best.fitness << std::endl;
        return best;
    }

}