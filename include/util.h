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
    InstanceType currentInstance;
    InstanceType loadInstance();
    StringVector readDirectory();
    StringMatrix readFile(String path);
    void loadInstanceObject();
    InstanceType returnInstance();
    template <typename T, typename U> T transpose(T& mat);

    StringMatrix cask() { return currentInstance[CASK]; }
    StringMatrix passagem() { return currentInstance[PASSAGEM]; }
    StringMatrix rotas() { return currentInstance[ROTAS]; }
    StringMatrix rotas2() { return currentInstance[ROTAS2]; }

}


#endif