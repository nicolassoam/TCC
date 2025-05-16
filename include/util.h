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
#include <filesystem>
#include "enumTypedef.h"
#include "config.h"

using String = std::string;
using StringVector = std::vector<String>; 
using StringMatrix = std::vector<StringVector>;
using InstanceType = std::vector<StringMatrix>;

namespace util
{
    InstanceType loadInstance();
    StringVector readDirectory();
    StringMatrix readFile(String path, bool _transpose = true);
    std::map<String, std::vector<double>> mapODs(StringMatrix p);
    template <typename T, typename U> T transpose(T& mat);
    
    inline StringMatrix cask(InstanceType t) { return t[CASK]; }
    inline StringMatrix passagem(InstanceType t) { return t[PASSAGEM]; }
    inline StringMatrix rotas(InstanceType t) { return t[ROTAS]; }
    inline StringMatrix rotas2(InstanceType t) { return t[ROTAS2]; }
    inline StringMatrix aeronave(InstanceType t) { return t[AERONAVE]; }
    // Map a string Matrix so that from aircraft type a [] to destination b [] is the value of the price aka prices[a][b] returns a double
    StringMatrix mapDestinationToAircraftTicketPrice(StringMatrix& dataMatrix);
    InstanceType FlightLegs(StringMatrix& dataMatrix);
}


#endif