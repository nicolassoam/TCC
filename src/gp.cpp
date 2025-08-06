#include "gp.h"

#include <algorithm>
#include <execution>
namespace GP
{
    std::mt19937 rng(SEED);

    int tournament(const Population &population, int k){
        int bestIndex = rand() % POPULATION_SIZE;
        for(int i = 0; i < k; i++){
            int randomIndex = rand() % POPULATION_SIZE;
            if(population[randomIndex].fitness > population[bestIndex].fitness){
                bestIndex = randomIndex;
            }
        }
        return bestIndex;
    }

#pragma region AUXILIAR
    void cleanupSchedule(Individual& individual) {
        individual.schedule.erase(
            std::remove_if(individual.schedule.begin(), individual.schedule.end(),
                [](const Flight& trip) {
                    return trip.outboundPassengersPerFlight == 0;
                }),
            individual.schedule.end()
                    );
    }
    std::map<int, int> getTotalFleetRequired(const Individual& individual) 
    {
        std::map<int, int> fleetMap;
        for (int i = 0; i < AIRCRAFT_TYPES; ++i) {
            if (individual.allowedAircraft[i]) 
            {
                fleetMap[i] = calculateRequiredFleetSize(individual, i);
            }
        }
        return fleetMap;
    }
    int calculateRequiredFleetSize(const Individual& individual, int aircraftTypeId) 
    {
        if (individual.schedule.empty()) {
            return 0;
        }

        std::vector<int> balanceChange(TIME_WINDOWS, 0);

        for (const auto& trip : individual.schedule) {
            if (trip.aircraftTypeId == aircraftTypeId) 
            {
                balanceChange[trip.outboundWindow] -= trip.frequency;

                int arrivalWindow = trip.returnWindow + 1 % TIME_WINDOWS;
                balanceChange[arrivalWindow] += trip.frequency;
            }
        }

        int aircraftOutsideHub = 0;
        int maxAircraftOutside = 0;

        for (int w = 0; w < TIME_WINDOWS; ++w) 
        {
            aircraftOutsideHub -= balanceChange[w];

            if (aircraftOutsideHub > maxAircraftOutside) {
                maxAircraftOutside = aircraftOutsideHub;
            }
        }

        return maxAircraftOutside;
    }
    void repair(Individual& child, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes) 
    {
        int allowedCount = std::accumulate(child.allowedAircraft.begin(), child.allowedAircraft.end(), 0);
        while (allowedCount > MAX_ASSIGNED_TYPES) {
            int idx = std::uniform_int_distribution<>(0, AIRCRAFT_TYPES - 1)(rng);
            if (child.allowedAircraft[idx]) {
                child.allowedAircraft[idx] = false;
                allowedCount--;
            }
        }
        if (allowedCount == 0 && !aircraftTypes.empty()) {
            int idx = std::uniform_int_distribution<>(0, AIRCRAFT_TYPES - 1)(rng);
            child.allowedAircraft[idx] = true;
        }

        child.schedule.erase(std::remove_if(child.schedule.begin(), child.schedule.end(),
            [&](const Flight& op) {
                return !child.allowedAircraft[op.aircraftTypeId];
            }), child.schedule.end());

        std::vector<Flight> repairedSchedule;

        std::shuffle(child.schedule.begin(), child.schedule.end(), rng);

        std::vector<int> demandTracker(routes.size() * TIME_WINDOWS, -1);


        for (const auto& trip : child.schedule) {
            int returnRouteId = trip.outboundRouteId + 10;

            int outDemandNeeded = trip.frequency * trip.outboundPassengersPerFlight;
            int retDemandNeeded = trip.frequency * trip.returnPassengersPerFlight;

            int out_idx = trip.outboundRouteId * TIME_WINDOWS + trip.outboundWindow;
            int ret_idx = returnRouteId * TIME_WINDOWS + trip.returnWindow;

            if (demandTracker[out_idx] == -1) {
                demandTracker[out_idx] = routes[trip.outboundRouteId].demandPerWindow[trip.outboundWindow];
            }

            if (demandTracker[ret_idx] == -1) {
                demandTracker[ret_idx] = routes[returnRouteId].demandPerWindow[trip.returnWindow];
            }

            int outDemandAvailable = demandTracker[out_idx];
            int retDemandAvailable = demandTracker[ret_idx];

            if (outDemandAvailable >= outDemandNeeded && retDemandAvailable >= retDemandNeeded) {
                repairedSchedule.push_back(trip);
                demandTracker[out_idx] -= outDemandNeeded;
                demandTracker[ret_idx] -= retDemandNeeded;
            }
        }

        child.schedule = repairedSchedule;
    }

    void getServedDemand(const Individual& individual, std::vector<int>& servedDemand) {
        if (servedDemand.size() != NUM_ROUTES * TIME_WINDOWS) {
            servedDemand.assign(NUM_ROUTES * TIME_WINDOWS, 0);
        }
        else {
            std::fill(servedDemand.begin(), servedDemand.end(), 0);
        }

        for (const auto& trip : individual.schedule) {
            int outboundIndex = trip.outboundRouteId * TIME_WINDOWS + trip.outboundWindow;
            servedDemand[outboundIndex] += trip.frequency * trip.outboundPassengersPerFlight;

            int returnRouteId = trip.outboundRouteId + 10;
            int returnIndex = returnRouteId * TIME_WINDOWS + trip.returnWindow;
            servedDemand[returnIndex] += trip.frequency * trip.returnPassengersPerFlight;
        }
        return;
    }
#pragma endregion

    void evaluateIndividual(Individual& ind, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes)
    {
        double sum = 0;
        int allowedCount = std::accumulate(ind.allowedAircraft.begin(), ind.allowedAircraft.end(), 0);

        if (allowedCount == 0 || allowedCount > MAX_ASSIGNED_TYPES)
        {
            ind.fitness = -1.0e18;
            return;
        }

        for (const auto& op : ind.schedule) 
        {
            int returnRouteId = op.outboundRouteId + 10;

            const auto& routeOut = routes[op.outboundRouteId];
            const auto& routeIn = routes[returnRouteId];
            const auto& aircraft = aircraftTypes[op.aircraftTypeId];
            double priceOut = routeOut.ticketPrices.at(aircraft.id);
            double priceIn = routeIn.ticketPrices.at(aircraft.id);
            double caskOut = routeOut.caskValues.at(aircraft.id);
            double caskIn = routeIn.caskValues.at(aircraft.id);

            double outboundRevenue = op.outboundPassengersPerFlight * priceOut;
            double returnRevenue = op.returnPassengersPerFlight * priceIn;

            double outboundCost = routeOut.distanceKM * aircraft.capacity * caskOut * op.frequency;
            double returnCost = routeIn.distanceKM * aircraft.capacity * caskIn * op.frequency;

            double cost = (outboundCost + returnCost);
            double profit = (outboundRevenue + returnRevenue) - cost;
            sum += profit;
        }
        ind.fitness = sum;
    }

    Population initializePopulation(const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes)
    {
        Population pop(POPULATION_SIZE);

        for (auto& individual : pop) 
        {
            int numToAllow = std::uniform_int_distribution<>(1, MAX_ASSIGNED_TYPES)(rng);
            std::vector<int> p(AIRCRAFT_TYPES);
            std::iota(p.begin(), p.end(), 0);
            std::shuffle(p.begin(), p.end(), rng);
            for (int i = 0; i < numToAllow; ++i) {
                individual.allowedAircraft[p[i]] = true;
            }
            int minMutationAmount = 10;
            int maxMutationAmount = 40;
            int initialOps = std::uniform_int_distribution<>(minMutationAmount, maxMutationAmount)(rng);
            for (int i = 0; i < initialOps; ++i) { mutate(individual, routes, aircraftTypes); }
        }

        return pop;
    }

#pragma region OPERATORS
    void mutateAdjustPassengers(Individual& individual, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes)
    {
        if (individual.schedule.empty()) {
            return;
        }

        int trip_idx = std::uniform_int_distribution<>(0, individual.schedule.size() - 1)(rng);
        Flight& tripToModify = individual.schedule[trip_idx];
        std::vector<int> servedDemand;
        getServedDemand(individual, servedDemand);

        const auto& aircraft = aircraftTypes[tripToModify.aircraftTypeId];

        int returnRouteId = tripToModify.outboundRouteId + 10;
        const auto& routeOut = routes[tripToModify.outboundRouteId];
        int totalDemandOut = routeOut.demandPerWindow[tripToModify.outboundWindow];
        int outIndex = tripToModify.outboundRouteId * TIME_WINDOWS + tripToModify.outboundWindow;
        int servedDemandOut = servedDemand[outIndex];
        int demandHeadroomOut = totalDemandOut - (servedDemandOut - tripToModify.frequency * tripToModify.outboundPassengersPerFlight);

        int maxPassengersOut = std::min(aircraft.capacity,
            demandHeadroomOut / tripToModify.frequency);

        const auto& routeIn = routes[returnRouteId];
        int totalDemandIn = routeIn.demandPerWindow[tripToModify.returnWindow];

        int returnIndex = returnRouteId * TIME_WINDOWS + tripToModify.returnWindow;

        int servedDemandIn = servedDemand[returnIndex];
        int demandHeadroomIn = totalDemandIn - (servedDemandIn - tripToModify.frequency * tripToModify.returnPassengersPerFlight);

        int maxPassengersIn = std::min(aircraft.capacity,
           demandHeadroomIn / tripToModify.frequency);

        if (maxPassengersOut > tripToModify.outboundPassengersPerFlight) {
            tripToModify.outboundPassengersPerFlight = std::uniform_int_distribution<>(
                tripToModify.outboundPassengersPerFlight, maxPassengersOut)(rng);
        }

        if (maxPassengersIn > tripToModify.returnPassengersPerFlight) {
            tripToModify.returnPassengersPerFlight = std::uniform_int_distribution<>(
                tripToModify.returnPassengersPerFlight, maxPassengersIn)(rng);
        }
    }


    void mutateAddRoute(Individual& individual, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes)
    {
        // Add route in Schedule
        std::vector<int> allowedAc;
        for (int i = 0; i < AIRCRAFT_TYPES; ++i) if (individual.allowedAircraft[i]) allowedAc.push_back(i);
        if (allowedAc.empty()) return;

        int acID = allowedAc[std::uniform_int_distribution<>(0, allowedAc.size() - 1)(rng)];

        int outRouteId = std::uniform_int_distribution<>(0, (routes.size() / 2) - 1)(rng);
        int retRouteId = outRouteId + 10;

        const auto& aircraft = aircraftTypes[acID];
        const auto& routeOut = routes[outRouteId];
        if (aircraft.rangeKM < routeOut.distanceKM) return;

        int outWindow = std::uniform_int_distribution<>(0, TIME_WINDOWS - 1)(rng);
        int validReturnWindows = -1;

        for (int i = 1; i < TIME_WINDOWS; ++i) {
            int potentialReturnWindow = (outWindow + i) % TIME_WINDOWS;

            if (routes[retRouteId].demandPerWindow[potentialReturnWindow] > 0) {
                validReturnWindows = potentialReturnWindow;
                break;
            }
        }

        if (validReturnWindows == -1) {
            return; 
        }

        int retWindow = validReturnWindows;

        int outPax = 0;
        int retPax = 0;
        int freq = 1;
        if (MAX_ASSIGNED_TYPES == 1)
        {
            int minOutPax = 0;
            int minRetPax = 0;
            if (routeOut.demandPerWindow[outWindow] > 0)
            {
                minOutPax = routeOut.demandPerWindow[outWindow] * 20 / 100;
            }
            if (routes[retRouteId].demandPerWindow[retWindow] > 0)
            {
                minRetPax = routes[retRouteId].demandPerWindow[retWindow] * 20 / 100;
            }
            outPax = std::uniform_int_distribution<>(minOutPax, routeOut.demandPerWindow[outWindow])(rng);
            retPax = std::uniform_int_distribution<>(minRetPax, routes[retRouteId].demandPerWindow[retWindow])(rng);
            int freqOut = (outPax + aircraft.capacity - 1) / aircraft.capacity;
            int freqIn = (retPax + aircraft.capacity - 1) / aircraft.capacity;
            freq = freqOut;
            if (freqIn > freq)
            {
                freq = freqIn;
            }
        }
        else
        {
            outPax = std::min(aircraft.capacity, routeOut.demandPerWindow[outWindow]);
            retPax = std::min(aircraft.capacity, routes[retRouteId].demandPerWindow[retWindow]);
        }
       
        if (outPax == 0) return;

        individual.schedule.push_back({ acID, outRouteId, outWindow, retWindow, freq, outPax, retPax });
    }
    void mutate(Individual& individual, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes)
    {
        int choice = std::uniform_int_distribution<>(0, 4)(rng);

        if (choice == 0) {
            // Flip Aircraft Type 
            int idx = std::uniform_int_distribution<>(0, AIRCRAFT_TYPES - 1)(rng);
            individual.allowedAircraft[idx] = !individual.allowedAircraft[idx];
            repair(individual,routes,aircraftTypes);
        }
        else if (choice == 1) 
        {
            mutateAddRoute(individual, routes, aircraftTypes);
            repair(individual, routes, aircraftTypes);
        }
        else if (choice == 2 && !individual.schedule.empty()) {
            // Remove route from schedule
            int idx = std::uniform_int_distribution<>(0, individual.schedule.size() - 1)(rng);
            individual.schedule.erase(individual.schedule.begin() + idx);
        }
        else if (choice == 3 && !individual.schedule.empty()) {
            // Decrease frequency from route in schedule
            int idx = std::uniform_int_distribution<>(0, individual.schedule.size() - 1)(rng);
            individual.schedule[idx].frequency += (std::uniform_int_distribution<>(0, 1)(rng) == 0) ? 1 : -1;
            if (individual.schedule[idx].frequency < 1) individual.schedule[idx].frequency = 1;
        }
        else if (choice == 4 && !individual.schedule.empty())
        {
            mutateAdjustPassengers(individual, routes, aircraftTypes);
        }
        cleanupSchedule(individual);
    }

    std::pair<Individual, Individual> crossover(const Individual& parent1, const Individual& parent2, const std::vector<AircraftType>& aircraftTypes, const std::vector<Route>& routes)
    {
        Individual child1, child2;
        float doCrossover = std::uniform_real_distribution<>(0, 1)(rng);
        if (doCrossover < CR)
        {
            int crossPtMask = std::uniform_int_distribution<>(0, AIRCRAFT_TYPES - 1)(rng);
            for (int i = 0; i < AIRCRAFT_TYPES; ++i) {
                child1.allowedAircraft[i] = (i < crossPtMask) ? parent1.allowedAircraft[i] : parent2.allowedAircraft[i];
                child2.allowedAircraft[i] = (i < crossPtMask) ? parent2.allowedAircraft[i] : parent1.allowedAircraft[i];
            }
        }
        else
        {
            child1.allowedAircraft = parent1.allowedAircraft;
            child2.allowedAircraft = parent2.allowedAircraft;
        }

        child1.schedule = parent1.schedule;
        child2.schedule = parent2.schedule;
        

        repair(child1, routes, aircraftTypes);
        repair(child2, routes, aircraftTypes);
        
        return { child1, child2 };
    }
#pragma endregion

#pragma region MAIN
    Individual search(const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes, 
        int generations, int populationSize, GPU::DeviceDataManager deviceData)
    {
        Population pop = initializePopulation(routes, aircraftTypes);

        for (int i = 0; i < pop.size();i++)
        {
            evaluateIndividual(pop[i], routes, aircraftTypes);
        }
        
        std::sort(std::execution::par_unseq, pop.begin(), pop.end(), [](const Individual& a, const Individual& b) {return a.fitness > b.fitness;});
        Individual best = pop[0];
        for (int gen = 0; gen < generations; ++gen) {
            Population newPopulation;
            newPopulation.push_back(best); 
            while (newPopulation.size() < POPULATION_SIZE)
            {
                int firstParentIdx = 0;
                int secondParentIdx = 0;
                do {
                    firstParentIdx = tournament(pop, TOURNAMENT_SIZE);
                    secondParentIdx = tournament(pop, TOURNAMENT_SIZE);
                } while (firstParentIdx == secondParentIdx);
                const Individual& parent1 = pop[firstParentIdx];
                const Individual& parent2 = pop[secondParentIdx];
                auto children = crossover(parent1, parent2, aircraftTypes, routes);
                float doMutation = std::uniform_real_distribution<>(0, 1.0)(rng);
                if(doMutation < MR)
                { 
                    mutate(children.first, routes, aircraftTypes); 
                    mutate(children.second, routes, aircraftTypes);
                }
                newPopulation.push_back(children.first);
                if (newPopulation.size() < POPULATION_SIZE) 
                {
                    newPopulation.push_back(children.second);
                }
            }
#ifndef GPU_ON
            for (auto& ind : newPopulation) { evaluateIndividual(ind,routes, aircraftTypes); }
#else
            GPU::kernelCaller(deviceData, newPopulation, gen);
#endif
            pop = newPopulation;
            auto bestCurrent = *std::max_element(pop.begin(), pop.end(),
                [](const auto& a, const auto& b) { return a.fitness < b.fitness; });
            if (bestCurrent.fitness > best.fitness) { best = bestCurrent; }
            std::cout << "Gen " << gen << ": Best Fitness = " << bestCurrent.fitness << " | Overall Best = " << best.fitness << std::endl;
        }
        return best;
    }  

}
#pragma endregion

namespace util
{

    struct SingleFlightLeg 
    {
        int timeWindow;
        int routeId;
        int aircraftTypeId;
        int frequency;
        int passengersPerFlight;
    };

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

    void writeFlightsPerDestination(const Individual& bestIndividual, const std::vector<Route>& routes)
    {
        std::map<std::string, int> destinationFrequencies;

        for (const auto& trip : bestIndividual.schedule) {
            const Route& outboundRoute = routes[trip.outboundRouteId];

            destinationFrequencies[outboundRoute.destinationIcao] += trip.frequency;
        }

#ifdef GPU_ON
        std::ofstream file(String(RESULT) + "/best_individualGPU" + std::to_string(MAX_ASSIGNED_TYPES) + "_flights_per_destination.txt");
#else
        std::ofstream file(String(RESULT) + "/best_individualCPU" + std::to_string(MAX_ASSIGNED_TYPES) + "_flights_per_destination.txt");
#endif

        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file for writing flight frequencies per destination." << std::endl;
            return;
        }

        file << "Destination_ICAO;Total_Frequency\n";

        for (const auto& pair : destinationFrequencies) {
            const std::string& destinationIcao = pair.first;
            int totalFrequency = pair.second;
            file << destinationIcao << ";" << totalFrequency << "\n";
        }

        file.close();
    }

    void writeSolutionTimes(std::vector<double>elapsedTimes)
    {
        std::ofstream file(String(RESULT) + "/times_solution" + std::to_string(MAX_ASSIGNED_TYPES) + ".txt", std::ios_base::app);
        double avgTime = 0;

        if (file.is_open())
        {
            for (int i = 0; i < elapsedTimes.size(); i++)
            {
                avgTime += elapsedTimes[i];
#ifdef GPU_ON
                file << "Time(s)(GPU) :" << elapsedTimes[i] << std::endl;
#else
                file << "Time(s) :" << elapsedTimes[i] << std::endl;
#endif
            }
            avgTime = avgTime / elapsedTimes.size();
#ifdef GPU_ON
            file << "Average Time per Solution(s) (GPU): " << avgTime << std::endl;
#else
            file << "Average Time per Solution(s): " << avgTime << std::endl;
#endif
        }
        file.close();
    }
    void writeBestIndividual(Individual& ind, InstanceType flightLegs, InstanceType instance, const std::vector<Route>& routes, const std::vector<AircraftType>& aircraftTypes)
    {
#ifdef GPU_ON
        std::ofstream file(String(RESULT)+"/best_individualGPU"+std::to_string(MAX_ASSIGNED_TYPES)+".csv");
#else
        std::ofstream file(String(RESULT)+"/best_individualCPU"+std::to_string(MAX_ASSIGNED_TYPES)+".csv");
#endif
        StringMatrix aircrafts = util::aeronave(instance);
        std::map<int, String> days;
        std::map<int, String> aircraftName;
        std::vector<std::map<int, String>> odPairs;
        std::vector<String> day = { "SEG", "TER", "QUA", "QUI", "SEX", "SAB", "DOM" };
        std::map<int, int> requiredFleet = GP::getTotalFleetRequired(ind);
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

        std::vector<SingleFlightLeg> all_flight_legs;
        for (const auto& trip : ind.schedule) {
            all_flight_legs.push_back({
                trip.outboundWindow,
                trip.outboundRouteId,
                trip.aircraftTypeId,
                trip.frequency,
                trip.outboundPassengersPerFlight
                });

            all_flight_legs.push_back({
                trip.returnWindow,
                trip.outboundRouteId + 10,
                trip.aircraftTypeId,
                trip.frequency,
                trip.returnPassengersPerFlight
                });
        }

        std::sort(all_flight_legs.begin(), all_flight_legs.end(),
            [](const SingleFlightLeg& a, const SingleFlightLeg& b) {
                if (a.timeWindow != b.timeWindow) {
                    return a.timeWindow < b.timeWindow; 
                }
        return a.routeId < b.routeId;
            }
        );

        if (file.is_open())
        {
            file << "Flight;Turn;Aircraft;Frequency;Passengers (" << ind.fitness << ")\n";
            for (const auto& op : all_flight_legs) {
                const Route& route = routes[op.routeId];
                const AircraftType& aircraft = aircraftTypes[op.aircraftTypeId];

                std::string od_pair = route.originIcao + route.destinationIcao;
                std::string turn_string = days.at(op.timeWindow);

                file << od_pair << ";"
                    << turn_string << ";"
                    << aircraft.name << ";"
                    << op.frequency << ";"
                    << op.passengersPerFlight << "\n";

            }
            file.close();
        }
        else
        {
            std::cerr << "Unable to open file";
        }
#ifdef GPU_ON
        std::ofstream fileFleet(String(RESULT) + "/best_individualGPU" + std::to_string(MAX_ASSIGNED_TYPES) + "aircraft_amount.txt");
#else
        std::ofstream fileFleet(String(RESULT) + "/best_individualCPU" + std::to_string(MAX_ASSIGNED_TYPES) + "aircraft_amount.txt");
#endif
        if (fileFleet.is_open())
        {
            for (const auto& pair : requiredFleet) {
                int aircraftId = pair.first;
                int fleetSize = pair.second;
                if (fleetSize > 0) 
                {
                    fileFleet << " Aircraft Type '" << aircraftTypes[aircraftId].name << "': "
                        << fleetSize << " planes" << std::endl;
                }
            }
            fileFleet.close();
        }
    }
}