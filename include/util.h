#ifndef UTIL_H_
#define UTIL_H_
#include <iostream>
#include <utility>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <queue>
#include <set>
#include <unordered_set>
#include <filesystem>
#include "enumTypedef.h"
#include "config.h"
#include <chrono>

#define MAX_ASSIGNED_TYPES 1
#define TIME_WINDOWS 28
#define NUM_ROUTES 20
#define HUB 0
#define POPULATION_SIZE 128
#define TOURNAMENT_SIZE 3
#define AIRCRAFT_TYPES 7
const float CR = 0.8;
const float MR = 0.4;
const int SEED = 320;

using String = std::string;
using StringVector = std::vector<String>; 
using StringMatrix = std::vector<StringVector>;
using InstanceType = std::vector<StringMatrix>;
using hClock = std::chrono::time_point <std::chrono::high_resolution_clock>;
struct AircraftType {
    int id;
    std::string name;
    int capacity;
    int rangeKM;
};

struct Route {
    int id;
    int originId;
    int destinationId;    
    String originIcao;
    String destinationIcao;
    double distanceKM;
    std::map<int,double> ticketPrices;
    std::map<int,double> caskValues;
    std::vector<int> demandPerWindow;
};

struct Flight
{
    int aircraftTypeId;
    int outboundRouteId;

    int outboundWindow;
    int returnWindow;

    int frequency;
    int outboundPassengersPerFlight;
    int returnPassengersPerFlight;
};

using FlightMatrix = std::vector<std::vector<Flight>>;
using FlightVector = std::vector<Flight>;

struct Individual
{
    float fitness;
    std::vector<Flight> schedule;
    std::vector<bool> allowedAircraft;
    Individual()
    {
        fitness = 0;
        allowedAircraft.resize(AIRCRAFT_TYPES);
    };
    ~Individual() {};
};

using Population = std::vector<Individual>;

namespace util
{
    InstanceType loadInstance();
    StringVector readDirectory();
    StringMatrix readFile(String path, bool _transpose = true);
    template <typename T, typename U> T transpose(T& mat);
    
    inline StringMatrix cask(InstanceType t) { return t[CASK]; }
    inline StringMatrix passagem(InstanceType t) { return t[PASSAGEM]; }
    inline StringMatrix rotas(InstanceType t) { return t[ROTAS]; }
    inline StringMatrix rotas2(InstanceType t) { return t[ROTAS2]; }
    inline StringMatrix aeronave(InstanceType t) { return t[AERONAVE]; }
    
    StringMatrix mapDestinationToAircraftTicketPrice(StringMatrix& dataMatrix, StringMatrix& flightMatrix);
    InstanceType FlightLegs(StringMatrix& dataMatrixFlight, StringMatrix& dataMatrixPass);
    std::vector<Route> readRoute(StringMatrix prices, StringMatrix cask, InstanceType flightData);
    std::vector<AircraftType> readAicrafts(StringMatrix aircrafts);
}


#endif