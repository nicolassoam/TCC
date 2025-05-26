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

    Population newGen(Population& population, InstanceType instance,float cr, float mr)
    {
        Population children;

        short firstParent = 0;
        short secondParent = 0;
        while(children.size() < POPULATION_SIZE)
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

    int constraintCheck(Individual& ind, InstanceType instance, InstanceType flightsMap)
    {
        StringMatrix flights = util::rotas2(instance);
        StringMatrix aircrafts = util::aeronave(instance);
        int penalty = 0;
        // pick vectors from individual
        std::vector<FlightMatrix>& flightData = ind.ch.flightData;
        //check constraints, if violated worst fitness
        int flightLegs = 10;
        int timeWindows = 28;
        int aircraftTypes = 7;
        int sum = 0;
        for(int i = 0; i < flightLegs; i++)
        {
            StringMatrix flight = flightsMap[i];
            for(int j = 0; j < timeWindows; j++)
            {
                StringVector flightTime = flight[j];
                int demand = std::stoi(flightTime[1]);
                for(int k = 0; k < aircraftTypes; k++)
                {
                    StringVector aircraft = aircrafts[k];
                    int maxSeatCapacity = std::stoi(aircraft[ASSENTOS]);
                    int aircraftRange = std::stoi(aircraft[ALCANCE]);
                    int flightDistance = std::stoi(flightTime[Rotas2_3::CollumnsRotas2_3::DISTANCIA]);
                    sum += flightData[i][j][k].passengerNumber;
                    if(flightData[i][j][k].passengerNumber > flightData[i][j][k].flightFrequency*maxSeatCapacity )
                    {
                       penalty += CONSTRAINT_PEN; 
                    }

                    if(aircraftRange < flightDistance && flightData[i][j][k].flightFrequency > 0)
                    {
                        penalty += CONSTRAINT_PEN;
                    }

                    // if(flightData[i][j][k].flightFrequency > fleetSize[k])
                    // {
                    //     penalty += CONSTRAINT_PEN;
                    // }

                    if (flightData[i][j][k].flightFrequency > 0 && flightData[i][j][k].flightFrequency > BIG_M)
                    {
                        return CONSTRAINT_PEN;
                    }

                    if(flightData[i][j][k].flightFrequency > 0 && flightData[i][j][k].flightFrequency > BIG_M)
                    {
                        return CONSTRAINT_PEN;
                    }
                    // fleetSize[k]++;
                }
                if(sum > demand)
                {
                    penalty += CONSTRAINT_PEN;
                }
            }
        }

        sum = 0;

        // for(int l = 0; l < aircraftTypes; l++)
        // {
        //     sum+= aircraftTypeDecisionVariable1[l];
        // }

        // if(sum > MAX_ASSIGNED_TYPES)
        // {
        //     penalty += CONSTRAINT_PEN;
        // }
        // sum = 0;
        // for(int m = 0; m < flightLegs; m++)
        // {
        //     int flightDistance = std::stoi(flights[m][Rotas2_3::CollumnsRotas2_3::DISTANCIA]);
            
        //     for(int n = 0; n < aircraftTypes; n++)
        //     {
        //         int aircraftRange = std::stoi(aircrafts[n][ALCANCE]);
        //         sum += aircraftTypeDecisionVariable2[m][n];
        //         if(aircraftRange < aircraftTypeDecisionVariable2[m][n] * flightDistance)
        //         {
        //             penalty += CONSTRAINT_PEN;
        //         }
        //     }
        //     if(sum > MAX_ASSIGNED_MODELS)
        //     {
        //         penalty += CONSTRAINT_PEN;
        //     }
        // }

        return penalty;
    }

    Population initializePopulation(int populationSize, int aircraftTypes, int flightLegs, int timeWindows)
    {
        Population pop;
    
        for(int i = 0; i < populationSize; i++)
        {
            Individual ind = Individual(aircraftTypes, flightLegs, timeWindows);

            std::vector<FlightMatrix>& flightData = ind.ch.flightData;

            for(int k = 0; k < flightLegs; k++)
            {

                for(int m = 0; m < timeWindows; m++)
                {
                    for(int n = 0; n < aircraftTypes; n++)
                    {
                        flightData[k][m][n].passengerNumber = rand() % 30;
                        if(m > 0)
                        {
                            flightData[k][m][n].flightFrequency = flightData[k][m-1][n].flightFrequency;
                        }
                        else
                        {
                            flightData[k][m][n].flightFrequency = rand() % 30;
                        }
                    }
                }

            }
            pop.push_back(ind);
        }

        return pop;
    }

    void evaluateIndividual (Individual& ind, InstanceType instance, 
                             StringMatrix flightLegPrices, StringMatrix caskValues, InstanceType flightsMap)
    {
        StringMatrix flights = util::rotas2(instance);
        StringMatrix aircrafts = util::aeronave(instance);
        StringMatrix cask = util::cask(instance);
        // pick vectors from individual
        std::vector<FlightMatrix>& flightData = ind.ch.flightData;

        // Constraint check
        int constraint = constraintCheck(ind, instance, flightsMap);
        
        ind.fitness += constraint;
        

        int flightLegs = 10;
        int timeWindows = 28;
        int aircraftTypes = AIRCRAFT_TYPES;
        double sum = 0;
        for(int l = 0; l < flightLegs; l++)
        {
            std::vector<String> prices = flightLegPrices[l];
            std::vector<String> cask = caskValues[l];
            for(int w = 0; w < timeWindows; w++)
            {
                for(int a = 0; a < aircraftTypes; a++)
                {
                    double price = std::stod(prices[a]);
                    double caskValue = std::stod(cask[a]);
                    sum += price * flightData[l][w][a].passengerNumber - (caskValue * flightData[l][w][a].flightFrequency);
                }
            }
        }
        ind.fitness = sum;
        return;
    }

    void crossover(Individual& fstChild, Individual& sndChild)
    {
        //Single point crossover
        
        int crossoverPointFlight = rand() % fstChild.ch.flightData.size();


        for(int m = crossoverPointFlight; m < fstChild.ch.flightData.size(); m++)
        {
            int secondPoint = rand() % fstChild.ch.flightData[m].size();
            for(int n = secondPoint; n < fstChild.ch.flightData[m].size(); n++)
            {
                int thirdPoint = rand() % fstChild.ch.flightData[m][n].size();
                for (int o = thirdPoint; o < fstChild.ch.flightData[m][n].size(); o++)
                {
                    std::swap(fstChild.ch.flightData[m][n][o], sndChild.ch.flightData[m][n][o]);
                }
            }
        }
    }

    void mutate(Individual& ind)
    {
        // Mutate all variables
        int mutationPointFlight = rand() % ind.ch.flightData.size();

        // Mutate fleet size
    
        // Mutate flight frequency
        for(int i = 0; i < ind.ch.flightData[mutationPointFlight].size(); i++)
        {
            for(int j = 0; j < ind.ch.flightData[mutationPointFlight][i].size(); j++)
            {
                if(j > 0)
                {
                    ind.ch.flightData[mutationPointFlight][i][j] = ind.ch.flightData[mutationPointFlight][i][j-1];
                }
                else
                {
                    ind.ch.flightData[mutationPointFlight][i][j].flightFrequency = rand() % 30;
                }

                ind.ch.flightData[mutationPointFlight][i][j].passengerNumber = rand() % 30;

            }
        }


    }

    Individual search(InstanceType flightLegs, StringMatrix cask, StringMatrix passagem, InstanceType instance, int generations, int populationSize, float mr, float cr)
    {
        Population pop;
        int aircraftTypes = util::passagem(instance).size() - 2;
        // 150 counting the return flight
        int flightLegsSize = 10;
        // Four daily shifts, as per article
        int timeWindows = 28;

        pop = initializePopulation(populationSize, aircraftTypes, flightLegsSize, timeWindows);

        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            evaluateIndividual(pop[i], instance, passagem, cask, flightLegs);
        }

        std::sort(std::execution::par_unseq,pop.begin(), pop.end(), [](Individual& a, Individual& b){return a.fitness > b.fitness;});
        Individual best = pop[0];

        std::cout << "Best fitness: " << best.fitness << std::endl;

        for (int i = 0; i < generations; i++)
        {
            Population children = newGen(pop, instance, mr, cr);
            // worst fitness first
            std::sort(std::execution::par_unseq,pop.begin(), pop.end(), [](Individual& a, Individual& b){return a.fitness < b.fitness;});
            pop[0] = best;
            std::sort(std::execution::par_unseq,pop.begin(), pop.end(), [](Individual& a, Individual& b){return a.fitness > b.fitness;});

            if(pop[0].fitness > best.fitness)
            {
                best = pop[0];
            }
            std::cout << "Generation: " << i << " Best fitness: " << best.fitness << std::endl;
        }

        std::cout << "Best fitness: " << best.fitness << std::endl;
        return best;
    }

}