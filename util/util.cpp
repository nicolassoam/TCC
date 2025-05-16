#include "util.h"

namespace util
{
    namespace fs = std::filesystem;
    template <typename T, typename U> 
    T transpose(T& mat) 
    {
        int rows = mat.size();
        int cols = mat[0].size();
        T res;
        

        res.resize(cols, U(rows));

        // Fill res with transposed values of mat
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                res[j][i] = mat[i][j];
            }
        }
        return res;
    }

    StringVector readDirectory()
    {

        StringVector v;
        for (const auto & entry : fs::directory_iterator(PROCESSED)){
            v.push_back(entry.path().string());
        }
        std::sort(v.begin(), v.end(),[](String& a, const String& b) {
            return a < b;
        });
        return v;

    }

    StringMatrix mapDestinationToAircraftTicketPrice(StringMatrix& dataMatrix)
    {
        StringMatrix pricesPerDestinationOnAircraftType;
        for(int i = 0; i < dataMatrix.size(); i++)
        {
            StringVector passagemRow = dataMatrix[i];
            StringVector pricesRow;
            for(int j = CaskPassagem::E190_E2; j < passagemRow.size(); j++)
            {
                String price = passagemRow[j];
                pricesRow.push_back(price);
            }
            pricesPerDestinationOnAircraftType.push_back(pricesRow);
        }
        return pricesPerDestinationOnAircraftType;
    }

    InstanceType FlightLegs(StringMatrix& dataMatrix)
    {
        InstanceType flights;
        int destinationCount = 11;
        std::set<String>doneDestinations;
        for(int i = 0; i < destinationCount; i++)
        {
            StringMatrix flightData;
            String currentDestination = " ";
            for(int j = 0; j < dataMatrix.size(); j++)
            {
               if(doneDestinations.find(dataMatrix[j][CaskPassagem::DESTINO]) == doneDestinations.end())
               {
                    currentDestination = dataMatrix[j][CaskPassagem::DESTINO];
                    doneDestinations.insert(currentDestination);
                }
            }
            for(int k = 0; k < dataMatrix.size(); k++)
            {
                StringVector flightLeg;
                StringVector flightLegData = dataMatrix[k];
                String flightLegDestination = flightLegData[CaskPassagem::DESTINO];
                if(flightLegDestination == currentDestination)
                {
                    flightLeg.push_back(flightLegData[Rotas2_3::PISTA]);
                    flightLeg.push_back(flightLegData[Rotas2_3::DEMANDA]);
                    flightLeg.push_back(flightLegData[Rotas2_3::DISTANCIA]);
                    flightData.push_back(flightLeg);
                }

            }
            if(!flightData.empty())
            {
                flights.push_back(flightData);
            }
            else
            {
                std::cerr << "No flight data found for destination: " << currentDestination << std::endl;
            }

        }
        return flights;
    }

    StringMatrix readFile(String path, bool _transpose)
    {
        std::ifstream file;
        String line;
        StringMatrix csv;
        try
        {
            file.open(path, std::ios::in);
            if (!file.is_open())
            {
                std::cerr << "Error opening file: " << path << std::endl;
                throw std::runtime_error("File not found");
            }
            getline(file, line); // header
            std::stringstream ss(line);
            String value;
            while(getline(file,line))
            {
                std::stringstream ss(line);
                String value;
                StringVector csvLine;
                while (getline(ss, value, ';'))
                {
                    csvLine.push_back(value);
                }
                csv.push_back(csvLine);
            }
        } 
        catch (const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
            return StringMatrix();
        }
        file.close();
        if(_transpose)
        {
            csv = transpose<StringMatrix, StringVector>(csv);
        }
        return csv;
    }

    std::map<String, std::vector<double>> mapODs(StringMatrix p)
    {
        std::map<String, std::vector<double>> m;
        int rows = p.size();

        for (int i = 0; i < rows; i++)
        {
            String flightLeg = p[i][CaskPassagem::DESTINO];
            std::vector<double> prices;
            for (int j = 2; j < p[i].size(); j++)
            {
                prices.push_back(std::stod(p[i][j]));
            }
            m.insert(std::pair<String, std::vector<double>>(flightLeg, prices));
        }

        return m;
    }

    InstanceType loadInstance()
    {
        StringVector instances = readDirectory();
        InstanceType instanceMatrix;

        /* AERONAVE */
        String aeronavePath = instances[AERONAVE];
        StringMatrix aeronave = readFile(aeronavePath, false);

        /* CASK */
        String caskPath = instances[CASK];
        StringMatrix cask = readFile(caskPath, false);

        /* PASSAGEM */
        String passagemPath = instances[PASSAGEM];
        StringMatrix passagem = readFile(passagemPath, false);

        /* ROTAS */
        String rotasPath = instances[ROTAS];
        StringMatrix rotas = readFile(rotasPath, false);

        /* ROTAS2 */
        String rotas2Path = instances[ROTAS2];
        StringMatrix rotas2 = readFile(rotas2Path, false);
        /*for (auto j : instances)
            std::cout << j << std::endl;*/
        instanceMatrix.push_back(aeronave);
        instanceMatrix.push_back(cask);
        instanceMatrix.push_back(passagem);
        instanceMatrix.push_back(rotas);
        instanceMatrix.push_back(rotas2);
        return instanceMatrix;
    }

} // namespace util