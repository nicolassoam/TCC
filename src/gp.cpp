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
        StringMatrix aircrafts = util::aeronave(instance);
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
                mutate(aircrafts,fstChild);
                mutate(aircrafts,sndChild);
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
        std::vector<FlightMatrix>& flightData = ind.ch.flightData;
        std::vector<int>& allowedAircraft = ind.ch.allowedAircraft;
        //check constraints, if violated worst fitness
        int flightLegs = 20;
        int timeWindows = 28;
        int aircraftTypes = 7;
        int sum = 0;
        for(int i = 0; i < flightLegs; i++)
        {
            StringMatrix flight;
            if (i < 10)
            {
                flight = flightsMap[i];
            }
            else
            {
                flight = flightsMap[i - 10];
            }
            for(int j = 0; j < timeWindows; j++)
            {
                StringVector flightTime = flight[j];
                int demand = std::stoi(flightTime[1]);
                for(int k = 0; k < aircraftTypes; k++)
                {
                    StringVector aircraft = aircrafts[k];
                    int maxSeatCapacity = std::stoi(aircraft[ASSENTOS]);
                    int aircraftRange = std::stoi(aircraft[ALCANCE]);
                    double flightDistance = std::stod(flightTime[2]);

                    int& passengerNumber = flightData[i][j][k].passengerNumber;
                    int& flightFrequency = flightData[i][j][k].flightFrequency;
                    int& allowCraft = allowedAircraft[k];

                    if (!allowCraft && flightFrequency > 0)
                    {
                        penalty += CONSTRAINT_PEN;
                    }

                    if (demand == 0)
                    {
                        passengerNumber = 0;
                        flightFrequency = 0;
                    }

                    sum += passengerNumber;
                    
                    float severity =  passengerNumber - (flightFrequency * maxSeatCapacity);
                    if (severity > 0)
                    {
                        penalty += CONSTRAINT_PEN * severity;
                    }
                    

                    if(aircraftRange < flightDistance && flightFrequency > 0)
                    {
                        penalty += CONSTRAINT_PEN;
                    }

                    // if(flightData[i][j][k].flightFrequency > fleetSize[k])
                    // {
                    //     penalty += CONSTRAINT_PEN;
                    // }

                    if (flightFrequency > 0 && flightFrequency > BIG_M)
                    {
                        penalty += CONSTRAINT_PEN;
                    }

                    if(flightFrequency == 0 && passengerNumber > 0)
                    {
                        penalty += CONSTRAINT_PEN;
                    }

                    // fleetSize[k]++;
                }
                  
                float severity = 0;
               
                severity = sum - demand;

                if(severity > 0)
                { 
                    penalty += CONSTRAINT_PEN * severity;
                }
                    
                sum = 0;
            }
        }

        return penalty;
    }

    Population initializePopulation(StringMatrix aircrafts,int populationSize, int aircraftTypes, int flightLegs, int timeWindows)
    {
        Population pop;

        for(int i = 0; i < populationSize; i++)
        {
            Individual ind = Individual(aircraftTypes, flightLegs, timeWindows);

            std::vector<FlightMatrix>& flightData = ind.ch.flightData;
            flightData.resize(flightLegs);
            for(int k = 0; k < flightLegs; k++)
            {

                for(int m = 0; m < timeWindows; m++)
                {
                    for(int n = 0; n < aircraftTypes; n++)
                    {
                        StringVector aircraft = aircrafts[n];
                        int maxSeatCapacity = std::stoi(aircraft[ASSENTOS]);

                        int& passengerNumber = flightData[k][m][n].passengerNumber;
                        int& flightFrequency = flightData[k][m][n].flightFrequency;
                        int& allowedAircraft = ind.ch.allowedAircraft[n];
                        allowedAircraft = rand() % 2;
                        
                        if (!allowedAircraft)
                        {
                            passengerNumber = 0;
                            flightFrequency = 0;
                            continue;
                        }
                            
                        int allowedAircraftCount = std::accumulate(ind.ch.allowedAircraft.begin(), ind.ch.allowedAircraft.end(), 0);
                        if (allowedAircraftCount > 3)
                        {
                            passengerNumber = rand() % 50;
                        }
                        else
                        {
                            passengerNumber = rand() % 134;
                        }

                        if(m < 27 && k>= 10)
                        {
                            flightFrequency = flightData[k - 10][m+1][n].flightFrequency;
                        }
                        else
                        {
                            flightFrequency = std::max(1,passengerNumber / maxSeatCapacity);
                            
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
        long constraint = constraintCheck(ind, instance, flightsMap);

        int flightLegs = 20;
        int timeWindows = 28;
        int aircraftTypes = AIRCRAFT_TYPES;
        double sum = 0;
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

                    int& flightFrequency = flightData[l][w][a].flightFrequency;
                    int& passengerNumber = flightData[l][w][a].passengerNumber;

                    if (passengerNumber == 0)
                    {
                        flightFrequency = 0;
                    }

                    if (flightFrequency > 0)
                    {
                        sum += price * passengerNumber - (caskValue * flightFrequency * flightDistance * maxSeatCapacity);
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
        int crossoverPointAirctaft = rand() % fstChild.ch.allowedAircraft.size();
        std::swap(fstChild.ch.flightData[crossoverPointFlight], sndChild.ch.flightData[crossoverPointFlight]);
        std::swap(fstChild.ch.allowedAircraft[crossoverPointAirctaft], sndChild.ch.allowedAircraft[crossoverPointAirctaft]);
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

    void mutate(StringMatrix aircrafts, Individual& ind)
    {
        int mutationPointFlight = rand() % ind.ch.flightData.size();
        int mutationPointAircraft = rand() % ind.ch.allowedAircraft.size();
        int &mutatedAircraft = ind.ch.allowedAircraft[mutationPointAircraft];
        int mutation = rand() % 2;

        if (mutatedAircraft != mutation && mutation == 0)
        {
            for (int i = 0; i < ind.ch.flightData.size(); i++)
            {
                for (int j = 0; j < ind.ch.flightData[i].size(); j++)
                {
                    int& passengerNumber = ind.ch.flightData[i][j][mutationPointAircraft].passengerNumber;
                    int& flightFrequency = ind.ch.flightData[i][j][mutationPointAircraft].flightFrequency;
                    passengerNumber = 0;
                    flightFrequency = 0;
                }
            }
        }

        // Mutate flight frequency
        for(int i = 0; i < ind.ch.flightData[mutationPointFlight].size(); i++)
        {
            for(int j = 0; j < ind.ch.flightData[mutationPointFlight][i].size(); j++)
            {
                int& passengerNumber = ind.ch.flightData[mutationPointFlight][i][j].passengerNumber;
                int& flightFrequency = ind.ch.flightData[mutationPointFlight][i][j].flightFrequency;
                int& allowAircraft = ind.ch.allowedAircraft[j];
                if (!allowAircraft)
                {
                   continue;
                }

                int allowedAircraftCount = std::accumulate(ind.ch.allowedAircraft.begin(), ind.ch.allowedAircraft.end(), 0);
                if (allowedAircraftCount > 3)
                {
                    passengerNumber = rand() % 50;
                }
                else
                {
                    passengerNumber = rand() % 134;
                }
                StringVector aircraft = aircrafts[j];
                int maxSeatCapacity = std::stoi(aircraft[ASSENTOS]);

                flightFrequency = std::max(1,passengerNumber / maxSeatCapacity);
                
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
        StringMatrix aicrafts = util::aeronave(instance);
        pop = initializePopulation(aicrafts, populationSize, aircraftTypes, flightLegsSize, timeWindows);

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
            pop = children;

            for (int j = 0; j < POPULATION_SIZE; j++)
            {
                evaluateIndividual(pop[j], instance, passagem, cask, flightLegs);
            }
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
    void sortBestFlight(Individual& best)
    {
        for (auto& flightMatrix : best.ch.flightData)
        {
            for (auto& timeWindow : flightMatrix)
            {
                std::sort(std::execution::par_unseq, timeWindow.begin(), timeWindow.end(),
                          [](const Flight& a, const Flight& b) { return a.passengerNumber > b.passengerNumber; });
            }
        }
    }

    std::vector<String> fourDaysShift(String day)
    {
        std::vector<String> fdayShift;

        for (int i = 0; i < 4; i++)
        {
            String dayShift = day + std::to_string(i+1);
            fdayShift.push_back(dayShift);
        }
        return fdayShift;
    }

    void writeBestIndividual(Individual& ind, InstanceType flightLegs, InstanceType instance)
    {
        std::ofstream file(String(RESULT)+"/best_individual.csv");
        StringMatrix aircrafts = util::aeronave(instance);
        //sortBestFlight(ind);
        std::map<int, String> days;
        std::map<int, String> aircraftName;
        std::vector<std::map<int, String>> odPairs;
        std::vector<String> day = { "SEG", "TER", "QUA", "QUI", "SEX", "SAB", "DOM" };
       
        int dayCount = 0;
        for (int k = 0; k < 7; k++)
        {
            std::vector<String> dayShift = fourDaysShift(day[k]);
            for (int l = 0; l < 4; l++)
            {
                days.insert({ dayCount, dayShift[l] });
                dayCount++;
            }
        }

        for (int a = 0; a < 7; a++)
        {
            StringVector aircraft = aircrafts[a];
            String name = aircraft[0];
            aircraftName.insert({ a, name });
        }

        for (int l = 0; l < flightLegs.size();l++)
        {
            StringMatrix flight = flightLegs[l];
            std::map<int, String> od;
            for (int m = 0; m < flight.size(); m++)
            {
                StringVector f = flight[m];
                od.insert({ m, f[3] });
            }
            odPairs.push_back(od);
        }

        int passengerSum = 0;
        if (file.is_open())
        {
            file << "Flight;Turn;Aircraft;Frequency;Passengers (" << ind.fitness << ")\n";
            for(int i = 0; i < ind.ch.flightData.size(); i++) 
            {
                for(int j = 0; j < ind.ch.flightData[i].size(); j++)
                {
                    for(int k = 0; k < ind.ch.flightData[i][j].size(); k++) 
                    {
                        passengerSum += ind.ch.flightData[i][j][k].passengerNumber;
                        if(i < 10)
                        { 
                            file << odPairs[i][j] << ";" << days[j] << ";" << aircraftName[k] << ";"
                                    << ind.ch.flightData[i][j][k].flightFrequency 
                                    << ";" << ind.ch.flightData[i][j][k].passengerNumber << std::endl;
                        }
                        else
                        {
                            String newOd = odPairs[i - 10][j].substr(4,7) + "SBGO";
                            file << newOd << ";" << days[j] << ";" << aircraftName[k] << ";"
                                << ind.ch.flightData[i][j][k].flightFrequency
                                << ";" << ind.ch.flightData[i][j][k].passengerNumber << std::endl;
                        }
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