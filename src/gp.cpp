#include "gp.h"
#include <algorithm>
#include <execution>
namespace GP
{

    int tournament(Population population, int k){
        int best_index = rand() % POPULATION_SIZE;
        for(int i = 0; i < k; i++){
            int random_index = rand() % POPULATION_SIZE;
            if(population[random_index].fitness > population[best_index].fitness){
                best_index = random_index;
            }
        }
        return best_index;
    }
    
    Population newGen(Population& population, InstanceType flightsMap, InstanceType instance,float cr, float mr, const StringMatrix& passagem, const StringMatrix& cask)
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

            Individual fstParent = population[firstParent];
            Individual sndParent= population[secondParent];
            Individual fstChild = fstParent;
            Individual sndChild = sndParent;
            if(rand()%100 < cr * 100)
            {
                std::pair<Individual,Individual>children = crossover(fstParent, sndParent, aircrafts, flightsMap);
                fstChild = children.first;
                sndChild = children.second;
            }
            if(rand()%100 < mr * 100)
            {
                mutate(fstChild, instance, aircrafts, flightsMap, passagem, cask);
                mutate(sndChild, instance, aircrafts, flightsMap, passagem, cask);
            }

            children.push_back(fstChild);
            if (children.size() < POPULATION_SIZE)
            {
                children.push_back(sndChild);
            }
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
        std::vector<Aircraft>& allowedAircraft = ind.ch.allowedAircraft;
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
                    Aircraft& allowCraft = allowedAircraft[k];

                    if (demand == 0)
                    {
                        flightFrequency = 0;
                        passengerNumber = 0;
                        bool isReturn = i >= 10;
                        if (isReturn)
                        {
                            int prevTurn = j == 0 ? 27 : j - 1;
                            int& outFreq = flightData[i - 10][prevTurn][k].flightFrequency;
                            int& outPassenger = flightData[i - 10][prevTurn][k].passengerNumber;
                            outFreq = 0;
                            outPassenger = 0;
                        }
                        else
                        {
                            int nextTurn = j == 27 ? 0 : j + 1;
                            int& inFreq = flightData[i + 10][nextTurn][k].flightFrequency;
                            int& inPassenger = flightData[i + 10][nextTurn][k].passengerNumber;
                            inFreq = 0;
                            inPassenger = 0;
                        }
                    }

                    if (!allowCraft.allowed && flightFrequency > 0)
                    {
                        penalty += CONSTRAINT_PEN;
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

                    /*if(flightData[i][j][k].flightFrequency > allowCraft.count)
                    {
                        penalty += CONSTRAINT_PEN;
                    }*/

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

    Population initializePopulation(InstanceType flightsMap, StringMatrix aircrafts,int populationSize, int aircraftTypes, int flightLegs, int timeWindows)
    {
        Population pop;

        for(int i = 0; i < populationSize; i++)
        {
            Individual ind = Individual(aircraftTypes, flightLegs, timeWindows);

            std::vector<FlightMatrix>& flightData = ind.ch.flightData;
            flightData.resize(flightLegs);
            bool firstModel = false;
            do
            {
                for (int a = 0; a < aircraftTypes; a++)
                {
                    Aircraft& allowedAircraft = ind.ch.allowedAircraft[a];
                    allowedAircraft.allowed = rand() % 2;

                    int allowedAircraftCount = std::accumulate(ind.ch.allowedAircraft.begin(), ind.ch.allowedAircraft.end(), 0,
                        [](int sum, const Aircraft& curr) { return sum + curr.allowed; });
                    if (allowedAircraft.allowed)
                    {
                        firstModel = true;
                    }
                    if (allowedAircraftCount == MAX_ASSIGNED_TYPES)
                    {
                        break;
                    }
                }
            } while (!firstModel);
            for(int k = 0; k < flightLegs; k++)
            {
                StringMatrix flight;
                if (k >= 10)
                {
                    flight = flightsMap[k - 10];
                }
                else
                {
                    flight = flightsMap[k];
                }
                for(int m = 0; m < timeWindows; m++)
                {
                    StringVector flightTurn = flight[m];
                    int demand = std::stoi(flightTurn[1]);

                    for(int n = 0; n < aircraftTypes; n++)
                    {
                        StringVector aircraft = aircrafts[n];
                        int maxSeatCapacity = std::stoi(aircraft[ASSENTOS]);

                        int& passengerNumber = flightData[k][m][n].passengerNumber;
                        int& flightFrequency = flightData[k][m][n].flightFrequency;
                        Aircraft& allowedAircraft = ind.ch.allowedAircraft[n];

                        if (!allowedAircraft.allowed)
                        {
                            passengerNumber = 0;
                            flightFrequency = 0;
                            continue;
                        }
                        else
                        {
                            allowedAircraft.count = rand() % 59 + 1;
                        }
                        if (demand == 0)
                        {
                            passengerNumber = 0;
                            flightFrequency = 0;
                            continue;
                        }
                        passengerNumber = rand() % demand ;
                        int remainingDemand = demand - passengerNumber;

                        if (remainingDemand < 0)
                        {
                            passengerNumber = demand;
                            demand = 0;
                        }
                        else
                        {
                            demand = remainingDemand;
                        }

                        if(k>= 10)
                        {
                            int previousTurn = m == 0 ? 27 : m - 1;
                            
                            flightFrequency = flightData[k - 10][previousTurn][n].flightFrequency;
                            int capacityMisMatch = (passengerNumber + maxSeatCapacity - 1) / maxSeatCapacity;
                            if (capacityMisMatch > flightFrequency && flightData[k - 10][previousTurn][n].passengerNumber > 0)
                            {
                                flightFrequency = capacityMisMatch;
                                ind.ch.flightData[k - 10][previousTurn][n].flightFrequency = capacityMisMatch;
                            }
                            else
                            {
                                passengerNumber = 0;
                                flightFrequency = 0;
                            }
                        }
                        else
                        {
                            flightFrequency = (passengerNumber + maxSeatCapacity - 1)/maxSeatCapacity;
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
                        bool returnFlight = (l >= 10);
                        if (returnFlight)
                        {
                            int previousTurn = w == 0 ? 27 : w - 1;

                            int& outFrequency = flightData[l - 10][previousTurn][a].flightFrequency;
                            int& outPassenger = flightData[l - 10][previousTurn][a].passengerNumber;
                            outFrequency = 0;
                            outPassenger = 0;
                        }
                        else
                        {
                            int nextTurn = w == 27 ? 0 : w + 1;
                            int& inFrequency = flightData[l + 10][nextTurn][a].flightFrequency;
                            int& inPassenger = flightData[l + 10][nextTurn][a].passengerNumber;
                            inFrequency = 0;
                            inPassenger = 0;
                        }
                    }

                    if (flightFrequency > 0)
                    {
                        sum += price * passengerNumber - (caskValue * flightFrequency * flightDistance * maxSeatCapacity);
                    }
                }
            }
        }
        // Constraint check
        long constraint = constraintCheck(ind, instance, flightsMap);
        ind.fitness = sum+constraint;
        return;
    }

    std::pair<Individual, Individual> crossover(const Individual& fstMate, const Individual& sndMate, const StringMatrix& aircrafts, const InstanceType& flightsMap)
    {
        int totalAircraftTypes = fstMate.ch.allowedAircraft.size();
        int flightLegs = fstMate.ch.flightData.size();
        int timeWindows = fstMate.ch.flightData[0].size();

        Individual fstChild(totalAircraftTypes, flightLegs, timeWindows);
        Individual sndChild(totalAircraftTypes, flightLegs, timeWindows);

        for (int a = 0; a < totalAircraftTypes; ++a) {
            if (rand() % 2 == 1) {
                fstChild.ch.allowedAircraft[a].allowed = sndMate.ch.allowedAircraft[a].allowed;
                sndChild.ch.allowedAircraft[a].allowed = fstMate.ch.allowedAircraft[a].allowed;
            }
            else
            {
                fstChild.ch.allowedAircraft[a].allowed = fstMate.ch.allowedAircraft[a].allowed;
                sndChild.ch.allowedAircraft[a].allowed = sndMate.ch.allowedAircraft[a].allowed;
            }

        }
        adequateAircraftAmount(fstChild);
        adequateAircraftAmount(sndChild);

        for (int l = 0; l < flightLegs; ++l)
        {
            if (rand() % 2 == 1) {
                fstChild.ch.flightData[l] = fstMate.ch.flightData[l];
                sndChild.ch.flightData[l] = sndMate.ch.flightData[l];
            }
            else {
                fstChild.ch.flightData[l] = sndMate.ch.flightData[l];
                sndChild.ch.flightData[l] = fstMate.ch.flightData[l];
            }
        }

        for (int a = 0; a < totalAircraftTypes; a++)
        {
            if (fstChild.ch.allowedAircraft[a].allowed)
            {
                fixFlippedAircraft(fstChild, a, aircrafts, flightsMap);
            }
            if (sndChild.ch.allowedAircraft[a].allowed)
            {
                fixFlippedAircraft(sndChild, a, aircrafts, flightsMap);
            }
        }

        /*for (int l = 0; l < flightLegs / 2; ++l)
        {
            for (int w = 0; w < timeWindows; ++w)
            {
                repairReturnFlightFrequencies(fstChild, l, w);
                repairReturnFlightFrequencies(sndChild, l, w);
            }
        }*/
        repairIndividual(fstChild);
        repairIndividual(sndChild);
        return { fstChild, sndChild };
    }
    void repairIndividual(Individual& ind)
    {
        int flightLegs = ind.ch.flightData.size();
        int timeWindows = ind.ch.flightData[0].size();
        int aircraftTypes = ind.ch.allowedAircraft.size();

        for (int a = 0; a < aircraftTypes; ++a) {
            if (!ind.ch.allowedAircraft[a].allowed) 
            {
                for (int l = 0; l < flightLegs; ++l) 
                {
                    for (int w = 0; w < timeWindows; ++w) 
                    {
                        int& passengerNumber = ind.ch.flightData[l][w][a].passengerNumber;
                        int& flightFrequency = ind.ch.flightData[l][w][a].flightFrequency;
                        passengerNumber = 0;
                        flightFrequency = 0;
                    }
                }
            }
        }

        for (int l = 0; l < flightLegs / 2; ++l) 
        { 
            for (int w = 0; w < timeWindows; ++w) 
            {
                repairReturnFlightFrequencies(ind, l, w);
            }
        }
    }

    void repairReturnFlightFrequencies(Individual& ind, int legIndex, int timeWindow)
    {
        bool isReturnFlight = legIndex >= 10;
        int aircraftTypes = ind.ch.flightData[0][0].size();

        for (int k = 0; k < aircraftTypes; ++k)
        {
            if (isReturnFlight) 
            {
                int correspondingOutboundLeg = legIndex - 10;
                int previousTimeWindow = (timeWindow == 0) ? 27 : timeWindow - 1;
                int& outBoundFlightFrequency = ind.ch.flightData[correspondingOutboundLeg][previousTimeWindow][k].flightFrequency;
                int& outBoundPassengers = ind.ch.flightData[correspondingOutboundLeg][previousTimeWindow][k].passengerNumber;
                outBoundFlightFrequency = ind.ch.flightData[legIndex][timeWindow][k].flightFrequency;
                if (outBoundPassengers == 0 && outBoundFlightFrequency > 0)
                {
                    outBoundPassengers = ind.ch.flightData[legIndex][timeWindow][k].passengerNumber;
                }
            }
            else 
            {
                int correspondingReturnLeg = legIndex + 10;
                int nextTimeWindow = (timeWindow == 27) ? 0 : timeWindow + 1;
                int& returnFlightFrequency = ind.ch.flightData[correspondingReturnLeg][nextTimeWindow][k].flightFrequency;
                int& returnFlightPassengers = ind.ch.flightData[correspondingReturnLeg][nextTimeWindow][k].passengerNumber;
                returnFlightFrequency = ind.ch.flightData[legIndex][timeWindow][k].flightFrequency;
                if (returnFlightPassengers == 0 && returnFlightFrequency > 0)
                {
                    returnFlightPassengers = ind.ch.flightData[legIndex][timeWindow][k].passengerNumber;
                }
            }
        }
    }

    void mutateAdjustPassengers(Individual& ind, const InstanceType& instance, const StringMatrix& aircrafts, const InstanceType& flightsMap)
    {
        int flightLegs = ind.ch.flightData.size();
        int timeWindows = ind.ch.flightData[0].size();
        int aircraftTypes = ind.ch.flightData[0][0].size();

        for (int i = 0; i < 20; ++i) { 
            int l = rand() % flightLegs;
            int w = rand() % timeWindows;
            int a = rand() % aircraftTypes;
            int& passengerNumRef = ind.ch.flightData[l][w][a].passengerNumber;
            int& frequencyRef = ind.ch.flightData[l][w][a].flightFrequency;
            if (passengerNumRef > 0) {
                int currentPax = ind.ch.flightData[l][w][a].passengerNumber;

                StringMatrix flight = (l < 10) ? flightsMap[l] : flightsMap[l - 10];
                int demand = std::stoi(flight[w][1]);

                int totalPaxInSlot = 0;
                for (const auto& flight : ind.ch.flightData[l][w]) {
                    totalPaxInSlot += flight.passengerNumber;
                }

                int demandHeadroom = demand - totalPaxInSlot;

                int delta = (rand() % (currentPax / 5 + 2)) - (currentPax / 10 + 1);
                int finalDelta = std::min(delta, demandHeadroom);
                int newPax = currentPax + finalDelta;

                if (newPax <= 0) {
                    passengerNumRef = 0;
                    frequencyRef = 0;
                }
                else {
                    passengerNumRef = newPax;
                    int capacity = std::stoi(aircrafts[a][ASSENTOS]);
                    int newFreq = (newPax + capacity - 1) / capacity;
                    frequencyRef = newFreq;
                }

                repairReturnFlightFrequencies(ind, l, w);
                return;
            }
        }
    }

    void mutateRemoveFlight(Individual& ind)
    {
        int flightLegs = ind.ch.flightData.size();
        int timeWindows = ind.ch.flightData[0].size();
        int aircraftTypes = ind.ch.flightData[0][0].size();

        for (int i = 0; i < 10; ++i) {
            int l = rand() % flightLegs;
            int w = rand() % timeWindows;
            int a = rand() % aircraftTypes;

            if (ind.ch.flightData[l][w][a].passengerNumber > 0) {
                ind.ch.flightData[l][w][a].passengerNumber = 0;
                ind.ch.flightData[l][w][a].flightFrequency = 0;
                repairReturnFlightFrequencies(ind, l, w);
                return;
            }
        }
    }

    void mutateAddFlight(Individual& ind, const InstanceType& instance, const StringMatrix& aircrafts, const InstanceType& flightsMap)
    {
        int flightLegs = ind.ch.flightData.size();
        int timeWindows = ind.ch.flightData[0].size();

        for (int i = 0; i < 20; ++i) 
        { 
            int l = rand() % flightLegs;
            int w = rand() % timeWindows;

            bool emptyFlight = true;
            for (const auto& flight : ind.ch.flightData[l][w]) {
                if (flight.passengerNumber > 0)
                {
                    emptyFlight = false;
                    break;
                };
            }

            if (emptyFlight) {
                StringMatrix flight = (l < 10) ? flightsMap[l] : flightsMap[l - 10];
                int demand = std::stoi(flight[w][1]);

                if (demand > 0) {
                    std::vector<int> allowedIndices;
                    for (int k = 0; k < ind.ch.allowedAircraft.size(); ++k) {
                        if (ind.ch.allowedAircraft[k].allowed) allowedIndices.push_back(k);
                    }

                    if (allowedIndices.empty()) return;

                    int aIndex = rand() % allowedIndices.size();
                    int a = allowedIndices[aIndex];

                    int passengersToAdd = 1 + (rand() % demand);
                    int capacity = std::stoi(aircrafts[a][ASSENTOS]);
                    
                    int frequency = (passengersToAdd + capacity - 1) / capacity;

                    ind.ch.flightData[l][w][a].passengerNumber = passengersToAdd;
                    ind.ch.flightData[l][w][a].flightFrequency = frequency;

                    repairReturnFlightFrequencies(ind, l, w);
                    return;
                }
            }
        }
    }

    void mutateSwapAircraft(Individual& ind, const StringMatrix& aircrafts)
    {
        int flightLegs = ind.ch.flightData.size();
        int timeWindows = ind.ch.flightData[0].size();

        for (int i = 0; i < 20; ++i) { 
            int l = rand() % flightLegs;
            int w = rand() % timeWindows;

            std::vector<int> operatingAircraftIndices;
            
            for (int k = 0; k < ind.ch.flightData[l][w].size(); ++k) {
                int passengerNumber = ind.ch.flightData[l][w][k].passengerNumber;
                if (passengerNumber > 0) operatingAircraftIndices.push_back(k);
            }

            if (operatingAircraftIndices.size() > 0) {
                int aFromIndex = rand() % operatingAircraftIndices.size();
                int aFrom = operatingAircraftIndices[aFromIndex];

                std::vector<int> allowedAndDifferentIndices;
                for (int k = 0; k < ind.ch.allowedAircraft.size(); ++k) {
                    if (ind.ch.allowedAircraft[k].allowed && k != aFrom) allowedAndDifferentIndices.push_back(k);
                }

                if (allowedAndDifferentIndices.empty()) return;

                int aToIndex = rand() % allowedAndDifferentIndices.size();
                int aTo = allowedAndDifferentIndices[aToIndex];

                int& passengerNumFrom = ind.ch.flightData[l][w][aFrom].passengerNumber;
                int& passengerNumTo = ind.ch.flightData[l][w][aTo].passengerNumber;
                int& frequencyFrom = ind.ch.flightData[l][w][aFrom].flightFrequency;
                int& frequencyTo = ind.ch.flightData[l][w][aTo].flightFrequency;
                passengerNumTo += passengerNumFrom;
                passengerNumFrom = 0;
                frequencyFrom = 0;

                int capacityTo = std::stoi(aircrafts[aTo][ASSENTOS]);
               
                frequencyTo = (passengerNumTo + capacityTo - 1) / capacityTo;

                repairReturnFlightFrequencies(ind, l, w);
                return;
            }
        }
    }

    void fixFlippedAircraft(Individual& ind, int flippedAircraft, const StringMatrix& aircrafts, const InstanceType& flightsMap)
    {
        StringVector aircraft = aircrafts[flippedAircraft];
        int allowedAircraftCount = std::accumulate(ind.ch.allowedAircraft.begin(), ind.ch.allowedAircraft.end(), 0,
            [](int sum, const Aircraft& curr) { return sum + curr.allowed; });
        for (int i = 0; i < ind.ch.flightData.size(); i++)
        {
            StringMatrix flight;
            if (i >= 10)
            {
                flight = flightsMap[i - 10];
            }
            else
            {
                flight = flightsMap[i];
            }
            for (int j = 0; j < ind.ch.flightData[i].size(); j++)
            {
                StringVector flightTurn = flight[j];
                int demand = std::stoi(flightTurn[1]);
                int maxSeatCapacity = std::stoi(aircraft[ASSENTOS]);
                int& passengerNumber = ind.ch.flightData[i][j][flippedAircraft].passengerNumber;
                int& flightFrequency = ind.ch.flightData[i][j][flippedAircraft].flightFrequency;
                if(allowedAircraftCount > 1)
                { 
                    for (int k = 0; k < ind.ch.flightData[i][j].size(); k++)
                    {
                        Aircraft& allowedAircraft = ind.ch.allowedAircraft[k];
                        if (!allowedAircraft.allowed)
                        {
                            continue;
                        }
                        else
                        {
                            int passengers = ind.ch.flightData[i][j][k].passengerNumber;
                            demand -= passengers;
                        }
                    }
                }

                Aircraft& allowedAircraft = ind.ch.allowedAircraft[flippedAircraft];
                if (!allowedAircraft.allowed)
                {
                    passengerNumber = 0;
                    flightFrequency = 0;
                    return;
                }
                else
                {
                    allowedAircraft.count = rand() % 59 + 1;
                }
                if (demand == 0)
                {
                    passengerNumber = 0;
                    flightFrequency = 0;
                    continue;
                }
                passengerNumber = rand() % demand;
                int remainingDemand = demand - passengerNumber;

                if (remainingDemand < 0)
                {
                    passengerNumber = demand;
                    demand = 0;
                }
                else
                {
                    demand = remainingDemand;
                }

                if (i >= 10)
                {
                    int previousTurn = j == 0 ? 27 : j - 1;

                    flightFrequency = ind.ch.flightData[i - 10][previousTurn][flippedAircraft].flightFrequency;
                    int capacityMisMatch = (passengerNumber + maxSeatCapacity - 1) / maxSeatCapacity;
                    if (capacityMisMatch > flightFrequency)
                    {
                        flightFrequency = capacityMisMatch;
                        ind.ch.flightData[i - 10][previousTurn][flippedAircraft].flightFrequency = capacityMisMatch;
                    }
                }
                else
                {
                    flightFrequency = (passengerNumber + maxSeatCapacity - 1) / maxSeatCapacity;
                }
                
            }
        }
    }
    void adequateAircraftAmount(Individual& ind)
    {
        int allowedAircraftCount = std::accumulate(ind.ch.allowedAircraft.begin(), ind.ch.allowedAircraft.end(), 0,
            [](int sum, const Aircraft& curr) { return sum + curr.allowed; });
        if (allowedAircraftCount <= MAX_ASSIGNED_TYPES && allowedAircraftCount > 0)
        {
            return;
        }
        int aircraftTypes = ind.ch.allowedAircraft.size();
        int aToFlip = 0;
        while (allowedAircraftCount == 0)
        {
            aToFlip = rand() % aircraftTypes;
            ind.ch.allowedAircraft[aToFlip].allowed = !ind.ch.allowedAircraft[aToFlip].allowed;
            allowedAircraftCount = std::accumulate(ind.ch.allowedAircraft.begin(), ind.ch.allowedAircraft.end(), 0,
                [](int sum, const Aircraft& curr) { return sum + curr.allowed; });
        }
        while (allowedAircraftCount > MAX_ASSIGNED_TYPES)
        {
            aToFlip = rand() % aircraftTypes;
            if(ind.ch.allowedAircraft[aToFlip].allowed)
            { 
                ind.ch.allowedAircraft[aToFlip].allowed = !ind.ch.allowedAircraft[aToFlip].allowed;
            }
            allowedAircraftCount = std::accumulate(ind.ch.allowedAircraft.begin(), ind.ch.allowedAircraft.end(), 0,
                [](int sum, const Aircraft& curr) { return sum + curr.allowed; });
        }
        return;
    }
    void mutateFlipAircraftType(Individual& ind, const StringMatrix& aircrafts, const InstanceType& flightsMap) {
        int aircraftTypes = ind.ch.allowedAircraft.size();
        if (aircraftTypes == 0) return;

        int aToFlip = rand() % aircraftTypes;
        ind.ch.allowedAircraft[aToFlip].allowed = !ind.ch.allowedAircraft[aToFlip].allowed;
        int allowedAircraftCount = std::accumulate(ind.ch.allowedAircraft.begin(), ind.ch.allowedAircraft.end(), 0,
            [](int sum, const Aircraft& curr) { return sum + curr.allowed; });
        while (allowedAircraftCount == 0)
        {
            aToFlip = rand() % aircraftTypes;
            ind.ch.allowedAircraft[aToFlip].allowed = !ind.ch.allowedAircraft[aToFlip].allowed;
            allowedAircraftCount = std::accumulate(ind.ch.allowedAircraft.begin(), ind.ch.allowedAircraft.end(), 0,
                [](int sum, const Aircraft& curr) { return sum + curr.allowed; });
        }
        while (allowedAircraftCount > MAX_ASSIGNED_TYPES)
        {
            // unflip until its fixed
            int unflip = 0;
            do
            { 
                unflip = rand() % aircraftTypes;
            } while ((unflip == aToFlip) || (!ind.ch.allowedAircraft[unflip].allowed));

            ind.ch.allowedAircraft[unflip].allowed = !ind.ch.allowedAircraft[unflip].allowed;

            allowedAircraftCount = std::accumulate(ind.ch.allowedAircraft.begin(), ind.ch.allowedAircraft.end(), 0,
                [](int sum, const Aircraft& curr) { return sum + curr.allowed; });
        }
        fixFlippedAircraft(ind, aToFlip, aircrafts, flightsMap);
        repairIndividual(ind);
    }
    void mutateHillClimb(Individual& ind, const InstanceType& instance, const StringMatrix& aircrafts, const InstanceType& flightsMap,
        const StringMatrix& passagem, const StringMatrix& cask)
    {
        Individual tempInd = ind; 

        mutateAdjustPassengers(tempInd, instance, aircrafts, flightsMap);

        evaluateIndividual(tempInd, instance, passagem, cask, flightsMap);

        if (tempInd.fitness >= ind.fitness) {
            ind = tempInd;
        }
    }
    void mutate(Individual& ind, const InstanceType& instance, const StringMatrix& aircrafts, const InstanceType& flightsMap, const StringMatrix& passagem, const StringMatrix& cask)
    {
        //double pHillClimb = 0.60; 
        double pAdjustPassengers = 0.50;
        double pAddFlight = 0.25;
        double pFlipAircraftType = 0.15;
        double pRemoveFlight = 0.05;
        double pSwapAircraft = 0.05;

        double r = static_cast<double>(rand()) / RAND_MAX;
       /* if (r < pHillClimb)
        {
            mutateHillClimb(ind, instance, aircrafts, flightsMap, passagem, cask);
        }*/
        if (r < pAdjustPassengers) {
            mutateAdjustPassengers(ind, instance, aircrafts, flightsMap);
        }
        else if (r < pAddFlight + pAdjustPassengers) {
            mutateAddFlight(ind, instance, aircrafts, flightsMap);
        }
        else if (r < pAddFlight + pFlipAircraftType + pAdjustPassengers) {
            mutateFlipAircraftType(ind, aircrafts, flightsMap);
        }
        else if (r < pAddFlight + pRemoveFlight + pAdjustPassengers + pSwapAircraft) {
            mutateRemoveFlight(ind);
        }
        else {
            mutateSwapAircraft(ind, aircrafts);
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
        StringMatrix aircrafts = util::aeronave(instance);
        pop = initializePopulation(flightLegs, aircrafts, populationSize, aircraftTypes, flightLegsSize, timeWindows);

        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            evaluateIndividual(pop[i], instance, passagem, cask, flightLegs);
        }

        std::sort(std::execution::par_unseq,pop.begin(), pop.end(), [](Individual& a, Individual& b){return a.fitness > b.fitness;});
        Individual best = pop[0];

        std::cout << "Best fitness: " << best.fitness << std::endl;

        for (int i = 0; i < generations; i++)
        {
            int firstParentIdx = 0;
            int secondParentIdx = 0;
            do {
                firstParentIdx = tournament(pop, TOURNAMENT_SIZE);
                secondParentIdx = tournament(pop, TOURNAMENT_SIZE);
            } while (firstParentIdx == secondParentIdx);
            Individual fstMate = pop[firstParentIdx];
            Individual sndMate = pop[secondParentIdx];

            auto childPair = crossover(fstMate, sndMate, aircrafts, flightLegs);
            Individual fstChild = childPair.first;
            Individual sndChild = childPair.second;

            mutate(fstChild, instance, aircrafts, flightLegs, passagem, cask);
            mutate(sndChild, instance, aircrafts, flightLegs, passagem, cask);

            evaluateIndividual(fstChild, instance, passagem, cask, flightLegs);
            evaluateIndividual(sndChild, instance, passagem, cask, flightLegs);


            pop[populationSize - 1] = fstChild;
            pop[populationSize - 2] = sndChild;

            std::sort(std::execution::par_unseq, pop.begin(), pop.end(), [](const Individual& a, const Individual& b) {return a.fitness > b.fitness;});

            if (pop[0].fitness > best.fitness) {
                best = pop[0];
            }

            std::cout << "Gen: " << i
                << " Best: " << best.fitness
                << " Current Best: " << pop[0].fitness
                << " Worst: " << pop[populationSize - 1].fitness << std::endl;
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