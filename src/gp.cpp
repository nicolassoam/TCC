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

    long constraintCheck(Individual& ind, InstanceType instance, InstanceType flightsMap)
    {
        StringMatrix flights = util::rotas2(instance);
        StringMatrix aircrafts = util::aeronave(instance);
        int penalty = 0;
        // pick vectors from individual
        
        //check constraints, if violated worst fitness
        int flightLegs = 20;
        int timeWindows = 28;
        int aircraftTypes = 7;
        int sum = 0;
        for(int a = 0; a < aircraftTypes; a++)
        {
            PassengerMatrix& flightData = ind.ch.aircraft[a].passengerPerFlight;
            StringVector aircraft = aircrafts[a];
            int maxSeatCapacity = std::stoi(aircraft[ASSENTOS]);
            int aircraftRange = std::stoi(aircraft[ALCANCE]);
            for(int l = 0; l < flightLegs; l++)
            {
                StringMatrix flight;
                if (l < 10)
                {
                    flight = flightsMap[l];
                }
                else
                {
                    flight = flightsMap[l - 10];
                }
                for(int w = 0; w < timeWindows; w++)
                {
                   StringVector flightTime = flight[w];
                   int demand = std::stoi(flightTime[1]);
                   double flightDistance = std::stod(flightTime[2]);
                   
                   if(flightData[l][w] > demand)
                   {
                       penalty += CONSTRAINT_PEN;
                   }

                   if(flightData[l][w] > maxSeatCapacity)
                   {
                       penalty += CONSTRAINT_PEN;
                   }
                   
                   if(aircraftRange < flightDistance && ind.ch.aircraft[a].count > 0)
                   {
                       penalty += CONSTRAINT_PEN;
                   }

                }
            }

        }

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

            for(int j = 0; j < aircraftTypes; j++)
            {
                PassengerMatrix& flightData = ind.ch.aircraft[j].passengerPerFlight;
                flightData.resize(flightLegs);
                for(int k = 0; k < flightLegs; k++)
                {
                    for(int l = 0; l < timeWindows; l++)
                    {
                        flightData[k][l] = rand() % 150;
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
        long constraint = constraintCheck(ind, instance, flightsMap);

        int flightLegs = 20;
        int timeWindows = 28;
        int aircraftTypes = AIRCRAFT_TYPES;
        double sum = 0;

        for(int a = 0; a < aircraftTypes; a++)
        {
            PassengerMatrix& flightDataAircraft = flightData[a].passengerPerFlight;
            for(int l = 0; l < flightLegs; l++)
            {
                StringMatrix flight;
                if (l < 10)
                {
                    flight = flightsMap[l];
                }
                else
                {
                    flight = flightsMap[l - 10];
                }
                for(int w = 0; w < timeWindows; w++)
                {
                    StringVector flightTime = flight[w];
                    int demand = std::stoi(flightTime[1]);
                    int passengerNumber = flightDataAircraft[l][w];
                    
                    double price = std::stod(flightLegPrices[l][a]);
                    sum += price * passengerNumber;
                }
            }
        }

        for(int l = 0; l < flightLegs; l++)
        {
            std::vector<String> prices;
            std::vector<String> cask;
            StringMatrix flight;

            if (l < 10)
            {
                prices = flightLegPrices[l];
                cask = caskValues[l];
                flight = flightsMap[l];
            }
            else
            {
                prices = flightLegPrices[l-10];
                cask = caskValues[l-10];
                flight = flightsMap[l - 10];
            }

            for(int w = 0; w < timeWindows; w++)
            {
                StringVector flightTime = flight[w];
                for(int a = 0; a < aircraftTypes; a++)
                {
                    StringVector aircraft = aircrafts[a];

                    double price = std::stod(prices[a]);
                    double caskValue = std::stod(cask[a]);
                    int maxSeatCapacity = std::stoi(aircraft[ASSENTOS]);

                    double flightDistance = std::stod(flightTime[2]);
                    if (flightData[l][w][a].flightFrequency > 0)
                    {
                        sum += price * flightData[l][w][a].passengerNumber - (caskValue * flightData[l][w][a].flightFrequency * flightDistance * maxSeatCapacity);
                    }
                }
            }
        }
        ind.fitness = sum+constraint;
        return;
    }

    void crossover(Individual& fstChild, Individual& sndChild)
    {
        //Single point crossover

        int crossoverPointFlight = rand() % fstChild.ch.flightData.size();

        std::swap(fstChild.ch.flightData[crossoverPointFlight], sndChild.ch.flightData[crossoverPointFlight]);
        /*for(int m = crossoverPointFlight; m < fstChild.ch.flightData.size(); m++)
        {
            int secondPoint = rand() % fstChild.ch.flightData[m].size();
            for(int n = secondPoint; n < fstChild.ch.flightData[m].size(); n++)
            {
                int thirdPoint = rand() % fstChild.ch.flightData[m][n].size();
                for (int o = thirdPoint; o < fstChild.ch.flightData[m][n].size(); o++)
                {
                    std::swap(fstChild.ch.flightData[m][n][o], sndChild.ch.flightData[m][n][o]);
                    if (m + 10 > 20)
                    {
                        if (n < 27)
                        {
                            fstChild.ch.flightData[m - 10][n + 1][o].flightFrequency = fstChild.ch.flightData[m][n][o].flightFrequency;
                        }
                    }
                    if (m + 10 < 20)
                    {
                        if (n > 0)
                        {
                            fstChild.ch.flightData[m + 10][n - 1][o].flightFrequency = fstChild.ch.flightData[m][n][o].flightFrequency;
                        }
                    }
                }
            }
            
        }*/
    }

    void mutate(Individual& ind)
    {
        int mutationPointFlight = rand() % ind.ch.flightData.size();
        // Mutate flight frequency
        for(int i = 0; i < ind.ch.flightData[mutationPointFlight].size(); i++)
        {
            for(int j = 0; j < ind.ch.flightData[mutationPointFlight][i].size(); j++)
            {
                ind.ch.flightData[mutationPointFlight][i][j].flightFrequency = (rand() % 50) + 1;
                if (mutationPointFlight + 10 > 20)
                {   
                    if (i < 27)
                    {
                        ind.ch.flightData[mutationPointFlight - 10][i + 1][j].flightFrequency = ind.ch.flightData[mutationPointFlight][i][j].flightFrequency;
                    }
                }

                if (mutationPointFlight + 10 < 20)
                {
                    if (i > 0)
                    {
                        ind.ch.flightData[mutationPointFlight + 10][i - 1][j].flightFrequency = ind.ch.flightData[mutationPointFlight][i][j].flightFrequency;
                    }
                }

                ind.ch.flightData[mutationPointFlight][i][j].passengerNumber = rand() % 150;

            }
        }


    }

    Individual search(InstanceType flightLegs, StringMatrix cask, StringMatrix passagem, InstanceType instance, int generations, int populationSize, float mr, float cr)
    {
        Population pop;
        int aircraftTypes = 7;
        // 150 counting the return flight
        int flightLegsSize = 20;
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

            /*for (int j = 0; j < POPULATION_SIZE;j++)
            {
                for (auto k : pop)
                {
                    std::cout << k.ch.flightData[0][0][0].flightFrequency << std::endl;
                }
            }*/

            std::cout << "Generation: " << i << " Best fitness: " << best.fitness << std::endl;
        }

        std::cout << "Best fitness: " << best.fitness << std::endl;
        return best;
    }

}

namespace util
{
    void writeBestIndividual(Individual& ind, InstanceType instance)
    {
        std::ofstream file(std::string(RESULT)+"/best_individual.txt");
        StringMatrix aircrafts = util::aeronave(instance);
        int passengerSum = 0;
        if (file.is_open())
        {
            file << "Fitness: " << ind.fitness << "\n";
            for(int i = 0; i < ind.ch.flightData.size(); i++) 
            {
                for(int j = 0; j < ind.ch.flightData[i].size(); j++)
                {
                    for(int k = 0; k < ind.ch.flightData[i][j].size(); k++) 
                    {
                        passengerSum += ind.ch.flightData[i][j][k].passengerNumber;
                        file << "Flight " << i << " Turn " << j << " Aircraft " << k << " - Frequency: " 
                                << ind.ch.flightData[i][j][k].flightFrequency 
                                << ", Passengers: " << ind.ch.flightData[i][j][k].passengerNumber << std::endl;
                    }
                }
            }
            file.close();
        }
        else
        {
            std::cerr << "Unable to open file";
        }
    }
}