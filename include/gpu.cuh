#pragma once
#include <iostream>
#include <stdio.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <vector>
#include <map>
#include "util.h"
#include <cooperative_groups.h>

namespace GPU
{
    using GPUFlight = Flight;
    struct GPUIndividual 
    {
        float fitness;

        int scheduleOffset; 
        int scheduleSize;   

        int allowedAircraftOffset; 
    };
	struct GPURoute
	{
        int id;
        int originId;
        int destinationId;
        double distanceKM;

        double* ticketPrices;
        double* caskValues;
        int* demandPerWindow;
	};

    struct GPUAircraftType 
    {
        int id;
        int capacity;
        int rangeKM;
    };
    struct DevicePopulationManager 
    {
        GPUIndividual* d_individuals = nullptr;

        GPUFlight* d_allFlights = nullptr;

        char* d_allAllowedAircraft = nullptr;

        int population_size;
        int totalFlights; 
    };
    struct DeviceDataManager 
    {
        GPUAircraftType* d_aircraftTypes = nullptr;
        GPURoute* d_routes = nullptr;

        double* d_allTicketPrices = nullptr;
        double* d_allCaskValues = nullptr;
        int* d_allDemands = nullptr;

        int numAircraftTypes;
        int numRoutes;
        int numTimeWindows;
        DeviceDataManager() {};
    };

    inline void cudaCheck(cudaError_t error) 
    {
        if (error != cudaSuccess) 
        {
            throw std::runtime_error("CUDA Error: " + std::string(cudaGetErrorString(error)));
        }
    }
    void setupDeviceData(DeviceDataManager& d_manager, const std::vector<AircraftType>& h_aircraftTypes, const std::vector<Route>& h_routes, int numTimeWindows);
    void cleanupDeviceData(DeviceDataManager& d_manager);
    void setupDevicePopulation(DevicePopulationManager& d_manager, const std::vector<Individual>& h_population);
    void cleanupDevicePopulation(DevicePopulationManager& d_manager);
    void kernelCaller(DeviceDataManager deviceData, Population& population, int currentGen);


}