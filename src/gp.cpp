#include "gp.h"
#include <algorithm>
#include <execution>
namespace GP
{
    std::mt19937 rng(std::random_device{}());
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
    
   
 /*   bool isFlowValid(const Individual& individual, std::vector<Route> routes)
    {
        const int num_airports = (routes.size() / 2) + 1;

        for (int ac_id = 0; ac_id < AIRCRAFT_TYPES; ++ac_id) {
            if (!individual.allowedAircraft[ac_id]) continue;

            std::vector<std::vector<int>> balance(num_airports, std::vector<int>(TIME_WINDOWS, 0));

            int required_fleet = 0;
            
            for (const auto& op : individual.schedule) {
                if (op.aircraft_type_id == ac_id && routes[op.route_id].origin_id == HUB) {
                    required_fleet += op.frequency;
                }
            }
            balance[HUB][0] = required_fleet;


            for (int w = 0; w < TIME_WINDOWS; ++w) {
                if (w > 0) {
                    for (int port_id = 0; port_id < num_airports; ++port_id) {
                        balance[port_id][w] += balance[port_id][w - 1];
                    }
                }

                for (const auto& op : individual.schedule) {
                    if (op.aircraft_type_id != ac_id || op.time_window != w) continue;

                    const auto& route = routes[op.route_id];
                    int origin = route.origin_id;
                    int dest = route.destination_id;
                    int freq = op.frequency;

                    if (balance[origin][w] < freq) {
                        return false; 
                    }
                    balance[origin][w] -= freq;

                    int arrival_window = (w + 1) % TIME_WINDOWS;
                    balance[dest][arrival_window] += freq;
                }
            }

            int final_balance = 0;
            for (int port_id = 0; port_id < num_airports; ++port_id) {
                final_balance += balance[port_id][TIME_WINDOWS - 1];
            }
            if (final_balance != required_fleet) {
                return false; 
            }
        }

        return true;
    }*/
    void evaluateIndividual(Individual& ind, std::vector<Route> routes, std::vector<AircraftType> aircraftTypes)
    {
        double sum = 0;
        int allowedCount = std::accumulate(ind.allowedAircraft.begin(), ind.allowedAircraft.end(), 0);
        if (allowedCount == 0 || allowedCount > MAX_ASSIGNED_TYPES)
        {
            ind.fitness = -1.0e18;
            return;
        }
        for (const auto& op : ind.schedule) {
            int return_route_id = op.outbound_route_id + 10;

            const auto& route_out = routes[op.outbound_route_id];
            const auto& route_in = routes[return_route_id];
            const auto& aircraft = aircraftTypes[op.aircraft_type_id];
            double price_out = route_out.ticket_prices.at(aircraft.id);
            double price_in = route_in.ticket_prices.at(aircraft.id);
            double cask_out = route_out.cask_values.at(aircraft.id);
            double cask_in = route_in.cask_values.at(aircraft.id);

            double outbound_revenue = op.outbound_passengers_per_flight * price_out;
            double return_revenue = op.return_passengers_per_flight * price_in;

            double outbound_cost = route_out.distance_km * aircraft.capacity * cask_out;
            double return_cost = route_in.distance_km * aircraft.capacity * cask_in;

            double cost = (outbound_cost + return_cost) * op.frequency;
            double profit = (outbound_revenue + return_revenue) - cost;
            sum += profit;
        }
        ind.fitness = sum;
    }

    Population initializePopulation(std::vector<Route> routes, std::vector<AircraftType> aircraftTypes)
    {
        Population pop(POPULATION_SIZE);

        for (auto& individual : pop) {
            int num_to_allow = std::uniform_int_distribution<>(1, MAX_ASSIGNED_TYPES)(rng);
            std::vector<int> p(AIRCRAFT_TYPES);
            std::iota(p.begin(), p.end(), 0);
            std::shuffle(p.begin(), p.end(), rng);
            for (int i = 0; i < num_to_allow; ++i) {
                individual.allowedAircraft[p[i]] = true;
            }

            int initial_ops = std::uniform_int_distribution<>(10, 40)(rng);
            for (int i = 0; i < initial_ops; ++i) { mutate(individual, routes, aircraftTypes); }
        }

        return pop;
    }

    

    std::pair<Individual, Individual> crossover(const Individual& parent1, const Individual& parent2, std::vector<AircraftType> aircraftTypes, std::vector<Route> routes)
    {
        Individual child1, child2;
        int cross_pt_mask = std::uniform_int_distribution<>(0, AIRCRAFT_TYPES - 1)(rng);
        for (int i = 0; i < AIRCRAFT_TYPES; ++i) {
            child1.allowedAircraft[i] = (i < cross_pt_mask) ? parent1.allowedAircraft[i] : parent2.allowedAircraft[i];
            child2.allowedAircraft[i] = (i < cross_pt_mask) ? parent2.allowedAircraft[i] : parent1.allowedAircraft[i];
        }
        child1.schedule = parent1.schedule;
        child2.schedule = parent2.schedule;
        repair(child1, routes, aircraftTypes);
        repair(child2, routes, aircraftTypes);
        return { child1, child2 };
    }
   
    void repair(Individual& child, std::vector<Route> routes, std::vector<AircraftType> aircraftTypes) {
        int allowed_count = std::accumulate(child.allowedAircraft.begin(), child.allowedAircraft.end(), 0);
        while (allowed_count > MAX_ASSIGNED_TYPES) {
            int idx = std::uniform_int_distribution<>(0, AIRCRAFT_TYPES - 1)(rng);
            if (child.allowedAircraft[idx]) {
                child.allowedAircraft[idx] = false;
                allowed_count--;
            }
        }
        if (allowed_count == 0 && !aircraftTypes.empty()) {
            int idx = std::uniform_int_distribution<>(0, AIRCRAFT_TYPES - 1)(rng);
            child.allowedAircraft[idx] = true;
        }

        child.schedule.erase(std::remove_if(child.schedule.begin(), child.schedule.end(),
            [&](const Flight& op) {
                return !child.allowedAircraft[op.aircraft_type_id];
            }), child.schedule.end());

        std::vector<Flight> repaired_schedule;

        std::map<std::pair<int, int>, int> demand_tracker; 

        std::shuffle(child.schedule.begin(), child.schedule.end(), rng);

        for (const auto& trip : child.schedule) {
            int return_route_id = trip.outbound_route_id + 10;

            int out_demand_needed = trip.frequency * trip.outbound_passengers_per_flight;
            int ret_demand_needed = trip.frequency * trip.return_passengers_per_flight;

            int out_demand_available = routes[trip.outbound_route_id].demand_per_window[trip.outbound_window];
            if (demand_tracker.count({ trip.outbound_route_id, trip.outbound_window })) {
                out_demand_available = demand_tracker.at({ trip.outbound_route_id, trip.outbound_window });
            }

            int ret_demand_available = routes[return_route_id].demand_per_window[trip.return_window];
            if (demand_tracker.count({ return_route_id, trip.return_window })) {
                ret_demand_available = demand_tracker.at({ return_route_id, trip.return_window });
            }

            if (out_demand_available >= out_demand_needed && ret_demand_available >= ret_demand_needed) {
                repaired_schedule.push_back(trip);

                demand_tracker[{trip.outbound_route_id, trip.outbound_window}] = out_demand_available - out_demand_needed;
                demand_tracker[{return_route_id, trip.return_window}] = ret_demand_available - ret_demand_needed;
            }
        }

        child.schedule = repaired_schedule;
    }
   
    void mutate(Individual& individual, std::vector<Route> routes, std::vector<AircraftType> aircraftTypes)
    {
        int choice = std::uniform_int_distribution<>(0, 4)(rng);

        if (choice == 0) {
            int idx = std::uniform_int_distribution<>(0, AIRCRAFT_TYPES - 1)(rng);
            individual.allowedAircraft[idx] = !individual.allowedAircraft[idx];
            repair(individual,routes,aircraftTypes);
        }
        else if (choice == 1) {
            std::vector<int> allowed_ac;
            for (int i = 0; i < AIRCRAFT_TYPES; ++i) if (individual.allowedAircraft[i]) allowed_ac.push_back(i);
            if (allowed_ac.empty()) return;

            int ac_id = allowed_ac[std::uniform_int_distribution<>(0, allowed_ac.size() - 1)(rng)];

            int out_route_id = std::uniform_int_distribution<>(0, (routes.size()/2) - 1)(rng);
            int ret_route_id = out_route_id + 10; 

            const auto& aircraft = aircraftTypes[ac_id];
            const auto& route_out = routes[out_route_id];
            if (aircraft.range_km < route_out.distance_km) return;

            int out_window = std::uniform_int_distribution<>(0, TIME_WINDOWS - 2)(rng);
            int ret_window = std::uniform_int_distribution<>(out_window + 1, TIME_WINDOWS - 1)(rng);

            int out_pax = std::min(aircraft.capacity, route_out.demand_per_window[out_window]);
            int ret_pax = std::min(aircraft.capacity, routes[ret_route_id].demand_per_window[ret_window]);
            if (out_pax == 0) return;

            individual.schedule.push_back({ ac_id, out_route_id, out_window, ret_window, 1, out_pax, ret_pax });
        }
        else if (choice == 2 && !individual.schedule.empty()) {
            int idx = std::uniform_int_distribution<>(0, individual.schedule.size() - 1)(rng);
            individual.schedule.erase(individual.schedule.begin() + idx);
        }
        else if (choice == 3 && !individual.schedule.empty()) {
            int idx = std::uniform_int_distribution<>(0, individual.schedule.size() - 1)(rng);
            individual.schedule[idx].frequency += (std::uniform_int_distribution<>(0, 1)(rng) == 0) ? 1 : -1;
            if (individual.schedule[idx].frequency < 1) individual.schedule[idx].frequency = 1;
        }
    }

    Individual search(std::vector<Route> routes, std::vector<AircraftType> aircraftTypes, int generations, int populationSize, float mr, float cr)
    {
        Population pop = initializePopulation(routes, aircraftTypes);
        for (int i = 0; i < pop.size();i++)
        {
            evaluateIndividual(pop[i], routes, aircraftTypes);
        }
        
        std::sort(std::execution::par_unseq, pop.begin(), pop.end(), [](const Individual& a, const Individual& b) {return a.fitness > b.fitness;});
        Individual best = pop[0];
        for (int gen = 0; gen < generations; ++gen) {
            Population new_population;
            new_population.push_back(best); 
            while (new_population.size() < POPULATION_SIZE)
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
                mutate(children.first, routes, aircraftTypes); 
                mutate(children.second, routes, aircraftTypes);
                new_population.push_back(children.first);
                if (new_population.size() < POPULATION_SIZE) 
                {
                    new_population.push_back(children.second);
                }
            }
            for (auto& ind : new_population) { evaluateIndividual(ind,routes, aircraftTypes); }
            pop = new_population;
            auto bestCurrent = *std::max_element(pop.begin(), pop.end(),
                [](const auto& a, const auto& b) { return a.fitness < b.fitness; });
            if (bestCurrent.fitness > best.fitness) { best = bestCurrent; }
            std::cout << "Gen " << gen << ": Best Fitness = " << bestCurrent.fitness << " | Overall Best = " << best.fitness << std::endl;
        }
        return best;
    }  

}

namespace util
{

    //void sortBestFlight(Individual& best)
    //{
    //    for (auto& flightMatrix : best.ch.flightData)
    //    {
    //        for (auto& timeWindow : flightMatrix)
    //        {
    //            std::sort(std::execution::par_unseq, timeWindow.begin(), timeWindow.end(),
    //                      [](const Flight& a, const Flight& b) { return a.passengerNumber > b.passengerNumber; });
    //        }
    //    }
    //}
    struct SingleFlightLeg {
        int time_window;
        int route_id;
        int aircraft_type_id;
        int frequency;
        int passengers_per_flight;
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

    void writeBestIndividual(Individual& ind, InstanceType flightLegs, InstanceType instance,std::vector<Route> routes, std::vector<AircraftType> aircraftTypes)
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

        std::vector<SingleFlightLeg> all_flight_legs;
        for (const auto& trip : ind.schedule) {
            all_flight_legs.push_back({
                trip.outbound_window,
                trip.outbound_route_id,
                trip.aircraft_type_id,
                trip.frequency,
                trip.outbound_passengers_per_flight
                });

            all_flight_legs.push_back({
                trip.return_window,
                trip.outbound_route_id + 10,
                trip.aircraft_type_id,
                trip.frequency,
                trip.return_passengers_per_flight
                });
        }

        std::sort(all_flight_legs.begin(), all_flight_legs.end(),
            [](const SingleFlightLeg& a, const SingleFlightLeg& b) {
                if (a.time_window != b.time_window) {
                    return a.time_window < b.time_window; 
                }
        return a.route_id < b.route_id;
            }
        );

        if (file.is_open())
        {
            file << "Flight;Turn;Aircraft;Frequency;Passengers (" << ind.fitness << ")\n";
            for (const auto& op : all_flight_legs) {
                // Look up details for this operation
                const Route& route = routes[op.route_id];
                const AircraftType& aircraft = aircraftTypes[op.aircraft_type_id];

                std::string od_pair = route.origin_icao + route.destination_icao;
                std::string turn_string = days.at(op.time_window);

                // Write the data row
                file << od_pair << ";"
                    << turn_string << ";"
                    << aircraft.name << ";"
                    << op.frequency << ";"
                    << op.passengers_per_flight << "\n";

            }
            file.close();
        }
        else
        {
            std::cerr << "Unable to open file";
        }
    }
}