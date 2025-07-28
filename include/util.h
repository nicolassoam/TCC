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

using String = std::string;
using StringVector = std::vector<String>; 
using StringMatrix = std::vector<StringVector>;
using InstanceType = std::vector<StringMatrix>;

struct AircraftType {
    int id;
    std::string name;
    int capacity;
    int range_km;
};

struct Route {
    int id;
    int origin_id;
    int destination_id;    
    String origin_icao;
    String destination_icao;
    double distance_km;
    std::map<int, double> ticket_prices;
    std::map<int, double> cask_values;
    std::vector<int> demand_per_window;
};

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