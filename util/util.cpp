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
            return a.length() < b.length();
        });
        return v;

    }

    StringMatrix readFile(String path)
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
        csv = transpose<StringMatrix, StringVector>(csv);
        return csv;
    }

    InstanceType loadInstance()
    {
        StringVector instances = readDirectory();
        InstanceType instanceMatrix;
        /* CASK */
        String caskPath = instances[CASK];
        StringMatrix cask = readFile(caskPath);

        /* PASSAGEM */
        String passagemPath = instances[PASSAGEM];
        StringMatrix passagem = readFile(passagemPath);

        /* ROTAS */
        String rotasPath = instances[ROTAS];
        StringMatrix rotas = readFile(rotasPath);

        /* ROTAS2 */
        String rotas2Path = instances[ROTAS2];
        StringMatrix rotas2 = readFile(rotas2Path);

        /* ROTAS3 */
        String rotas3Path = instances[ROTAS3];
        StringMatrix rotas3 = readFile(rotas3Path);

        instanceMatrix.push_back(cask);
        instanceMatrix.push_back(passagem);
        instanceMatrix.push_back(rotas);
        instanceMatrix.push_back(rotas2);
        instanceMatrix.push_back(rotas3);
        return instanceMatrix;
    }

    void loadInstanceObject()
    {
        currentInstance = returnInstance();
    }

    InstanceType returnInstance()
    {
        if (currentInstance.size() == 0)
        {
            loadInstanceObject();
        }
        return currentInstance;
    }

} // namespace util