#include "gp.h"
#include <algorithm>
#include <execution>
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

    Individual search(int generations, int populationSize, float mr, float cr)
    {
        Population pop;
        pop = initializePopulation(populationSize);
        evaluatePopulation();
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