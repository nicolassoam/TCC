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

    StringMatrix mapDestinationToAircraftTicketPrice(StringMatrix& dataMatrix, StringMatrix& flightMatrix)
    {
        StringMatrix pricesPerDestinationOnAircraftType;
        std::set<String>orderOfDestinations;

        int dataMatrixPassSize = dataMatrix.size();

        for (int i = 0; i < flightMatrix.size(); i++)
        {
            StringVector flightLine = flightMatrix[i];
            for (int j = 0; j < flightLine.size(); j++)
            {
                String destino = flightLine[Rotas2_3::DESTINO];
                orderOfDestinations.insert(destino);
            }
        }
        orderOfDestinations.erase("SBGO");
        while (!orderOfDestinations.empty())
        {
            for (int i = 0; i < dataMatrix.size(); i++)
            {
                StringVector passagemRow = dataMatrix[i];
                StringVector pricesRow;
                String destination = passagemRow[CaskPassagem::DESTINO];
                
                if (orderOfDestinations.find(destination) != orderOfDestinations.end())  
                {
                    orderOfDestinations.erase(destination);
                    for (int j = CaskPassagem::E190_E2; j < passagemRow.size(); j++)
                    {
                        String price = passagemRow[j];
                        pricesRow.push_back(price);
                    }
                    pricesPerDestinationOnAircraftType.push_back(pricesRow);
                    break;
                }
            }
        }
        return pricesPerDestinationOnAircraftType;
    }

    InstanceType FlightLegs(StringMatrix& dataMatrixFlight, StringMatrix& dataMatrixPass)
    {
        InstanceType flights;
        int destinationCount = 0;
        std::queue<String>orderOfDestinations;

        int dataMatrixPassSize = dataMatrixPass.size();

        for(int i = 0; i < dataMatrixPassSize; i++)
        {
            String destination = dataMatrixPass[i][CaskPassagem::DESTINO];

            orderOfDestinations.push(destination);

        }
        destinationCount = orderOfDestinations.size() + 1;
        for(int j = 0; j < destinationCount; j++)
        {
            StringMatrix flightData;
            String currentDestination = " ";
            if(orderOfDestinations.empty())
            {
                currentDestination = "SBGO";
            } 
            else
            {
                currentDestination = orderOfDestinations.front();
                orderOfDestinations.pop();
            }
            
            for(int k = 0; k < dataMatrixFlight.size(); k++)
            {
                StringVector flightLeg;
                StringVector flightLegData = dataMatrixFlight[k];
                String flightLegDestination = flightLegData[Rotas2_3::DESTINO];
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