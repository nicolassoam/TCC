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
        int tries = 0;
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

            int fstChildValidation = constraintCheck(fstChild, instance);
            int sndChildValidation = constraintCheck(sndChild, instance);

            if(fstChildValidation == 0)
            {
                children.push_back(fstChild);
            }
            else
            {
                tries++;
                if(tries >= MAX_TRIES)
                {
                    children.push_back(population[firstParent]);
                    tries = 0;
                }
            }
            
            if(sndChildValidation == 0)
            {
                children.push_back(sndChild);
            }
            else
            {
                tries++;
                if(tries >= MAX_TRIES)
                {
                    children.push_back(population[secondParent]);
                    tries = 0;
                }
            }

        }
        return children;
    }

    int constraintCheck(Individual& ind, InstanceType instance)
    {
        StringMatrix flights = util::rotas2(instance);
        StringMatrix aircrafts = util::aeronave(instance);
        // pick vectors from individual
        std::vector<int>& fleetSize = ind.ch.fleetSize;
        std::vector<int>& aircraftTypeDecisionVariable1 = ind.ch.aircraftTypeDecisionVariable1;
        IntMatrix& aircraftTypeDecisionVariable2 = ind.ch.aircraftTypeDecisionVariable2;
        std::vector<IntMatrix>& flightFrequency = ind.ch.flightFrequency;
        std::vector<IntMatrix>& passengerNumber = ind.ch.passengerNumber;

        //check constraints, if violated worst fitness
        int flightLegs = passengerNumber.size();
        int timeWindows = passengerNumber[0].size();
        int aircraftTypes = passengerNumber[0][0].size();
        int sum = 0;
        for(int i = 0; i < flightLegs; i++)
        {
            StringVector flight = flights[i];
            int demand = std::stoi(flight[Rotas2_3::CollumnsRotas2_3::DEMANDA]);
            for(int j = 0; j < timeWindows; j++)
            {
                for(int k = 0; k < aircraftTypes; k++)
                {
                    StringVector aircraft = aircrafts[k];
                    int maxSeatCapacity = std::stoi(aircraft[ASSENTOS]);
                    sum += passengerNumber[i][j][k];
                    if(passengerNumber[i][j][k] > flightFrequency[i][j][k]*maxSeatCapacity )
                    {
                       return CONSTRAINT_PEN; 
                    }

                    if(flightFrequency[i][j][k] > fleetSize[k])
                    {
                        return CONSTRAINT_PEN;
                    }
                    if(flightFrequency[i][j][k] > aircraftTypeDecisionVariable2[i][k] * BIG_M)
                    {
                        return CONSTRAINT_PEN;
                    }
                }
                if(sum > demand)
                {
                    return CONSTRAINT_PEN;
                }
            }
        }

        sum = 0;

        for(int l = 0; l < aircraftTypes; l++)
        {
            sum+= aircraftTypeDecisionVariable1[l];
        }

        if(sum > MAX_ASSIGNED_TYPES)
        {
            return CONSTRAINT_PEN;
        }
        sum = 0;
        for(int m = 0; m < flightLegs; m++)
        {
            int flightDistance = std::stoi(flights[m][Rotas2_3::CollumnsRotas2_3::DISTANCIA]);
            
            for(int n = 0; n < aircraftTypes; n++)
            {
                int aircraftRange = std::stoi(aircrafts[n][ALCANCE]);
                sum += aircraftTypeDecisionVariable2[m][n];
                if(aircraftRange < aircraftTypeDecisionVariable2[m][n] * flightDistance)
                {
                    return CONSTRAINT_PEN;
                }
            }
            if(sum > MAX_ASSIGNED_MODELS)
            {
                return CONSTRAINT_PEN;
            }
        }

        return 0;
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
                aircraftTypeDecisionVariable1.push_back(rand() % 2);
            }

            for(int k = 0; k < flightLegs; k++)
            {
                for(int l = 0; l < aircraftTypes; l++)
                {
                    aircraftTypeDecisionVariable2[k][l] = rand() % 2;
                }

                for(int m = 0; m < timeWindows; m++)
                {
                    for(int n = 0; n < aircraftTypes; n++)
                    {
                        flightFrequency[k][m][n] = rand() % 30;
                        passengerNumber[k][m][n] = rand() % 100;
                    }
                }

            }
            pop.push_back(ind);
        }

        return pop;
    }

    void evaluateIndividual (Individual& ind, InstanceType instance)
    {
        StringMatrix flights = util::rotas2(instance);
        StringMatrix aircrafts = util::aeronave(instance);
        // pick vectors from individual
        std::vector<int>& fleetSize = ind.ch.fleetSize;
        std::vector<int>& aircraftTypeDecisionVariable1 = ind.ch.aircraftTypeDecisionVariable1;
        IntMatrix& aircraftTypeDecisionVariable2 = ind.ch.aircraftTypeDecisionVariable2;
        std::vector<IntMatrix>& flightFrequency = ind.ch.flightFrequency;
        std::vector<IntMatrix>& passengerNumber = ind.ch.passengerNumber;

        // Constraint check
        int constraint = constraintCheck(ind, instance);
        if (constraint == CONSTRAINT_PEN)
        {
            ind.fitness = CONSTRAINT_PEN;
        }
    }

    void crossover(Individual& fstChild, Individual& sndChild)
    {
        //Single point crossover
        int crossoverPointFleet = rand() % fstChild.ch.fleetSize.size();
        int crossoverPointAircraftTypeBin1 = rand() % fstChild.ch.aircraftTypeDecisionVariable1.size();
        int crossoverPointAircraftTypeBin2 = rand() % fstChild.ch.aircraftTypeDecisionVariable2.size();
        int crossoverPointFlightFrequency = rand() % fstChild.ch.flightFrequency.size();
        int crossoverPointPassengerNumber = rand() % fstChild.ch.passengerNumber.size();

        for(int i = crossoverPointFleet; i < fstChild.ch.fleetSize.size(); i++)
        {
            std::swap(fstChild.ch.fleetSize[i], sndChild.ch.fleetSize[i]);
        }

        for(int j = crossoverPointAircraftTypeBin1; j < fstChild.ch.aircraftTypeDecisionVariable1.size(); j++)
        {
            std::swap(fstChild.ch.aircraftTypeDecisionVariable1[j], sndChild.ch.aircraftTypeDecisionVariable1[j]);
        }

        for(int k = crossoverPointAircraftTypeBin2; k < fstChild.ch.aircraftTypeDecisionVariable2.size(); k++)
        {
            int secondPoint = rand() % fstChild.ch.aircraftTypeDecisionVariable2[k].size();
            for(int l = secondPoint; l < fstChild.ch.aircraftTypeDecisionVariable2[k].size(); l++)
            {
                std::swap(fstChild.ch.aircraftTypeDecisionVariable2[k][l], sndChild.ch.aircraftTypeDecisionVariable2[k][l]);
            }
        }

        for(int m = crossoverPointFlightFrequency; m < fstChild.ch.flightFrequency.size(); m++)
        {
            int secondPoint = rand() % fstChild.ch.flightFrequency[m].size();
            for(int n = secondPoint; n < fstChild.ch.flightFrequency[m].size(); n++)
            {
                int thirdPoint = rand() % fstChild.ch.flightFrequency[m][n].size();
                for (int o = thirdPoint; o < fstChild.ch.flightFrequency[m][n].size(); o++)
                {
                    std::swap(fstChild.ch.flightFrequency[m][n][o], sndChild.ch.flightFrequency[m][n][o]);
                }
            }
        }

        for(int p = crossoverPointPassengerNumber; p < fstChild.ch.passengerNumber.size(); p++)
        {
            int secondPoint = rand() % fstChild.ch.passengerNumber[p].size();
            for(int q = secondPoint; q < fstChild.ch.passengerNumber[p].size(); q++)
            {
                int thirdPoint = rand() % fstChild.ch.passengerNumber[p][q].size();
                for (int r = thirdPoint; r < fstChild.ch.passengerNumber[p][q].size(); r++)
                {
                    std::swap(fstChild.ch.passengerNumber[p][q][r], sndChild.ch.passengerNumber[p][q][r]);
                }
            }
        }
    }

    void mutate(Individual& ind)
    {
        // Mutate all variables
        int mutationPointFleet = rand() % ind.ch.fleetSize.size();
        int mutationPointAircraftTypeBin1 = rand() % ind.ch.aircraftTypeDecisionVariable1.size();
        int mutationPointAircraftTypeBin2 = rand() % ind.ch.aircraftTypeDecisionVariable2.size();
        int mutationPointFlightFrequency = rand() % ind.ch.flightFrequency.size();
        int mutationPointPassengerNumber = rand() % ind.ch.passengerNumber.size();

        // Mutate fleet size

        ind.ch.fleetSize[mutationPointFleet] = rand() % 300 + 1;
        // Mutate aircraft type decision variable 1
        ind.ch.aircraftTypeDecisionVariable1[mutationPointAircraftTypeBin1] = rand() % 2;
        // Mutate aircraft type decision variable 2
        for(int i = 0; i < ind.ch.aircraftTypeDecisionVariable2[mutationPointAircraftTypeBin2].size(); i++)
        {
            ind.ch.aircraftTypeDecisionVariable2[mutationPointAircraftTypeBin2][i] = rand() % 2;
        }
        // Mutate flight frequency
        for(int i = 0; i < ind.ch.flightFrequency[mutationPointFlightFrequency].size(); i++)
        {
            for(int j = 0; j < ind.ch.flightFrequency[mutationPointFlightFrequency][i].size(); j++)
            {
                ind.ch.flightFrequency[mutationPointFlightFrequency][i][j] = rand() % 30;
            }
        }

        // Mutate passenger number
        for(int i = 0; i < ind.ch.passengerNumber[mutationPointPassengerNumber].size(); i++)
        {
            for(int j = 0; j < ind.ch.passengerNumber[mutationPointPassengerNumber][i].size(); j++)
            {
                ind.ch.passengerNumber[mutationPointPassengerNumber][i][j] = rand() % 100;
            }
        }

    }

    Individual search(InstanceType instance, int generations, int populationSize, float mr, float cr)
    {
        Population pop;
        int aircraftTypes = util::passagem(instance).size() - 2;
        // 150 counting the return flight
        int flightLegs = 150;
        // Four daily shifts, as per article
        int timeWindows = 4;
        
        pop = initializePopulation(populationSize, aircraftTypes, flightLegs, timeWindows);

        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            evaluateIndividual(pop[i], instance);
        }

        std::sort(std::execution::par_unseq,pop.begin(), pop.end(), [](Individual& a, Individual& b){return a.fitness < b.fitness;});
        Individual best = pop[0];

        std::cout << "Best fitness: " << best.fitness << std::endl;

        for (int i = 0; i < generations; i++)
        {
            Population children = newGen(pop, instance, mr, cr);
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