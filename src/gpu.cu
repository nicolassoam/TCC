#include "gpu.cuh"

namespace GPU
{
    void cleanupDeviceData(DeviceDataManager& d_manager) 
    {
        cudaFree(d_manager.d_aircraftTypes);
        cudaFree(d_manager.d_routes);
        cudaFree(d_manager.d_allTicketPrices);
        cudaFree(d_manager.d_allCaskValues);
        cudaFree(d_manager.d_allDemands);

        d_manager.d_aircraftTypes = nullptr;
        d_manager.d_routes = nullptr;
        d_manager.d_allTicketPrices = nullptr;
        d_manager.d_allCaskValues = nullptr;
        d_manager.d_allDemands = nullptr;
        d_manager.numAircraftTypes = 0;
        d_manager.numTimeWindows = 0;
        d_manager.numRoutes = 0;
    };

    void cleanupDevicePopulation(DevicePopulationManager& d_manager) 
    {
        cudaFree(d_manager.d_individuals);
        cudaFree(d_manager.d_allFlights);
        cudaFree(d_manager.d_allAllowedAircraft);

        d_manager.d_individuals = nullptr;
        d_manager.d_allFlights = nullptr;
        d_manager.d_allAllowedAircraft = nullptr;
    }
    void setupDevicePopulation(
        DevicePopulationManager& d_manager,
        const std::vector<Individual>& h_population)
    {
        if (h_population.empty()) {
            std::cerr << "Warning: Host population is empty." << std::endl;
            return;
        }

        d_manager.population_size = h_population.size();

        try {

            std::vector<GPUIndividual> h_gpuIndividuals;
            std::vector<GPUFlight>     h_allFlights;
            std::vector<char>          h_allAllowedAircraft;

            int currentFlightOffset = 0;
            int currentMaskOffset = 0;

            for (const auto& cpuInd : h_population) 
            {
                GPUIndividual gpuIndMeta;
                gpuIndMeta.fitness = cpuInd.fitness;

                gpuIndMeta.scheduleOffset = currentFlightOffset;
                gpuIndMeta.scheduleSize = cpuInd.schedule.size();

                gpuIndMeta.allowedAircraftOffset = currentMaskOffset;

                h_gpuIndividuals.push_back(gpuIndMeta);

                h_allFlights.insert(h_allFlights.end(), cpuInd.schedule.begin(), cpuInd.schedule.end());
                h_allAllowedAircraft.insert(h_allAllowedAircraft.end(), cpuInd.allowedAircraft.begin(), cpuInd.allowedAircraft.end());

                currentFlightOffset += cpuInd.schedule.size();
                currentMaskOffset += cpuInd.allowedAircraft.size();
            }

            d_manager.totalFlights = h_allFlights.size();

            // ========================================================================
            // Part B: Allocate memory on the GPU for the flattened arrays
            // ========================================================================

            cudaCheck(cudaMalloc(&d_manager.d_individuals, sizeof(GPUIndividual) * h_gpuIndividuals.size()));
            cudaCheck(cudaMalloc(&d_manager.d_allFlights, sizeof(GPUFlight) * h_allFlights.size()));
            cudaCheck(cudaMalloc(&d_manager.d_allAllowedAircraft, sizeof(char) * h_allAllowedAircraft.size()));

            // ========================================================================
            // Part C: Copy the flattened host vectors to the device
            // ========================================================================

            cudaCheck(cudaMemcpy(d_manager.d_individuals, h_gpuIndividuals.data(), sizeof(GPUIndividual) * h_gpuIndividuals.size(), cudaMemcpyHostToDevice));
            cudaCheck(cudaMemcpy(d_manager.d_allFlights, h_allFlights.data(), sizeof(GPUFlight) * h_allFlights.size(), cudaMemcpyHostToDevice));
            cudaCheck(cudaMemcpy(d_manager.d_allAllowedAircraft, h_allAllowedAircraft.data(), sizeof(char) * h_allAllowedAircraft.size(), cudaMemcpyHostToDevice));

        }
        catch (const std::exception& e) 
        {
            std::cerr << e.what() << std::endl;
        }
    }

    void setupDeviceData(DeviceDataManager& d_manager, const std::vector<AircraftType>& h_aircraftTypes,
        const std::vector<Route>& h_routes,int numTimeWindows)
    {
        d_manager.numAircraftTypes = h_aircraftTypes.size();
        d_manager.numRoutes = h_routes.size();
        d_manager.numTimeWindows = numTimeWindows;

        try {

            // 1. Flatten AircraftType data
            std::vector<GPUAircraftType> h_gpuAircraftTypes;
            for (const auto& ac : h_aircraftTypes) 
            {
                h_gpuAircraftTypes.push_back({ ac.id, ac.capacity, ac.rangeKM });
            }

            // 2. Flatten all sub-arrays (prices, casks, demands) into single large vectors
            std::vector<double> h_allTicketPrices;
            std::vector<double> h_allCaskValues;
            std::vector<int>    h_allDemands;

            for (const auto& route : h_routes) 
            {
                for (int i = 0; i < d_manager.numAircraftTypes; ++i) {
                    h_allTicketPrices.push_back(route.ticketPrices.at(i));
                    h_allCaskValues.push_back(route.caskValues.at(i));
                }
                h_allDemands.insert(h_allDemands.end(), route.demandPerWindow.begin(), route.demandPerWindow.end());
            }

            // ========================================================================
            // Part B: Allocate memory on the GPU
            // ========================================================================

            cudaCheck(cudaMalloc(&d_manager.d_aircraftTypes, sizeof(GPUAircraftType) * d_manager.numAircraftTypes));
            cudaCheck(cudaMalloc(&d_manager.d_routes, sizeof(GPURoute) * d_manager.numRoutes));

            cudaCheck(cudaMalloc(&d_manager.d_allTicketPrices, sizeof(double) * h_allTicketPrices.size()));
            cudaCheck(cudaMalloc(&d_manager.d_allCaskValues, sizeof(double) * h_allCaskValues.size()));
            cudaCheck(cudaMalloc(&d_manager.d_allDemands, sizeof(int) * h_allDemands.size()));


            cudaCheck(cudaMemcpy(d_manager.d_aircraftTypes, h_gpuAircraftTypes.data(), sizeof(GPUAircraftType) * d_manager.numAircraftTypes, cudaMemcpyHostToDevice));
            cudaCheck(cudaMemcpy(d_manager.d_allTicketPrices, h_allTicketPrices.data(), sizeof(double) * h_allTicketPrices.size(), cudaMemcpyHostToDevice));
            cudaCheck(cudaMemcpy(d_manager.d_allCaskValues, h_allCaskValues.data(), sizeof(double) * h_allCaskValues.size(), cudaMemcpyHostToDevice));
            cudaCheck(cudaMemcpy(d_manager.d_allDemands, h_allDemands.data(), sizeof(int) * h_allDemands.size(), cudaMemcpyHostToDevice));


            std::vector<GPURoute> h_gpuRoutes;
            for (int i = 0; i < d_manager.numRoutes; ++i) 
            {
                GPURoute tempRoute;
                tempRoute.id = h_routes[i].id;
                tempRoute.originId = h_routes[i].originId;
                tempRoute.destinationId = h_routes[i].destinationId;
                tempRoute.distanceKM = h_routes[i].distanceKM;

                tempRoute.ticketPrices = d_manager.d_allTicketPrices + (i * d_manager.numAircraftTypes);
                tempRoute.caskValues = d_manager.d_allCaskValues + (i * d_manager.numAircraftTypes);
                tempRoute.demandPerWindow = d_manager.d_allDemands + (i * d_manager.numTimeWindows);

                h_gpuRoutes.push_back(tempRoute);
            }

            cudaCheck(cudaMemcpy(d_manager.d_routes, h_gpuRoutes.data(), sizeof(GPURoute) * d_manager.numRoutes, cudaMemcpyHostToDevice));

        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }


    __global__ void evaluateFitnessKernel(
        GPUIndividual* d_individuals,
        GPUFlight* d_allFlights,
        char* d_allAllowedAircraft,
        int population_size,

        GPURoute* d_routes,
        GPUAircraftType* d_aircraftTypes,
        int numAircraftTypes,
        int routeOffset,
        int max_assigned_types)
    {
        int individual_idx = blockIdx.x * blockDim.x + threadIdx.x;

        if (individual_idx >= population_size) 
        {
            return;
        }

        GPUIndividual myIndMeta = d_individuals[individual_idx];
        GPUFlight* mySchedule = d_allFlights + myIndMeta.scheduleOffset;
        char* myAllowedMask = d_allAllowedAircraft + myIndMeta.allowedAircraftOffset;

        int allowedCount = 0;
        for (int i = 0; i < numAircraftTypes; ++i) 
        {
            if (myAllowedMask[i]) {
                allowedCount++;
            }
        }

        if (allowedCount == 0 || allowedCount > max_assigned_types) 
        {
            d_individuals[individual_idx].fitness = -1.0e18f; 
            return;
        }

        double totalProfit = 0.0; 
        for (int i = 0; i < myIndMeta.scheduleSize; ++i) 
        {
            GPUFlight flight = mySchedule[i];

            int returnRouteId = flight.outboundRouteId + routeOffset;

            const GPURoute& routeOut = d_routes[flight.outboundRouteId];
            const GPURoute& routeIn = d_routes[returnRouteId];
            const GPUAircraftType& aircraft = d_aircraftTypes[flight.aircraftTypeId];

            double priceOut = routeOut.ticketPrices[aircraft.id];
            double priceIn = routeIn.ticketPrices[aircraft.id];
            double caskOut = routeOut.caskValues[aircraft.id];
            double caskIn = routeIn.caskValues[aircraft.id];

            double outboundRevenue = flight.outboundPassengersPerFlight * priceOut;
            double returnRevenue = flight.returnPassengersPerFlight * priceIn;
            double outboundCost = routeOut.distanceKM * aircraft.capacity * caskOut * flight.frequency;
            double returnCost = routeIn.distanceKM * aircraft.capacity * caskIn * flight.frequency;

            double revenue = outboundRevenue + returnRevenue;
            double cost = (outboundCost + returnCost);

            double profitPerTrip = revenue - cost;
            totalProfit += profitPerTrip;
        }

        d_individuals[individual_idx].fitness = static_cast<float>(totalProfit);
    }


    void kernelCaller(DeviceDataManager deviceData, Population& population, int currentGen)
    {
        DevicePopulationManager devicePopulationManager;
        std::chrono::duration<double> elapsed;

        setupDevicePopulation(devicePopulationManager, population);

        int threadsPerBlock = 128;
        cudaDeviceProp props;
        int deviceId;
        cudaGetDevice(&deviceId);
        cudaGetDeviceProperties(&props, deviceId);
        int numSMs = props.multiProcessorCount;
        int blocksPerGrid = numSMs * 4;

        evaluateFitnessKernel << <blocksPerGrid, threadsPerBlock >> > (
            devicePopulationManager.d_individuals,
            devicePopulationManager.d_allFlights,
            devicePopulationManager.d_allAllowedAircraft,
            devicePopulationManager.population_size,
          
            deviceData.d_routes,
            deviceData.d_aircraftTypes,
            deviceData.numAircraftTypes,
            10, 
            MAX_ASSIGNED_TYPES   
            );
        
        
        cudaCheck(cudaGetLastError());
        cudaCheck(cudaDeviceSynchronize());
        std::vector<GPUIndividual> h_gpuIndividualsResult(devicePopulationManager.population_size);

        // Perform the copy from Device memory to Host memory

        cudaCheck(cudaMemcpy(
            h_gpuIndividualsResult.data(),                 
            devicePopulationManager.d_individuals,                 
            sizeof(GPUIndividual) * devicePopulationManager.population_size, 
            cudaMemcpyDeviceToHost                           
        ));

        for (int i = 0; i < population.size(); ++i) 
        {
            population[i].fitness = h_gpuIndividualsResult[i].fitness;
        }

        //cleanupDeviceData(deviceData);
        cleanupDevicePopulation(devicePopulationManager);
    };
}