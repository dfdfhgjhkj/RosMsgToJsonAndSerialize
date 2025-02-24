#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <cctype> 


std::string headerFileStr;

std::vector<std::string> typeVec = {"ColorRGBA","bool", "byte","int8", "int16", "int32", "int64","uint8", "uint16", "uint32", "uint64","float32", "float64","string","time","duration"};
std::vector<std::string> filePathVec;
bool isAllSpaces(const std::string& str) 
{
    return std::all_of(str.begin(), str.end(), 
        [](unsigned char c) 
        {
            return std::isspace(c);
        });
}
std::string replace(std::string str, std::string a, std::string b)
{
    int oldPos = 0;
    while (str.find(a, oldPos) != -1)
    {
        int start = str.find(a, oldPos);

        str.replace(start, a.size(), b);

        oldPos = start + b.size();
    }
    return str;
}

std::vector<std::string> split(const std::string& s, char delimiter) 
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void TraverseFolder(const std::string& folderPath)
{
    for (const auto& entry : std::filesystem::directory_iterator(folderPath))
    {
        //std::cout << entry.path() << std::endl;
        std::vector<std::string> pathStringVec= split(entry.path().string(), '.');
        if (pathStringVec.size()!=0&&pathStringVec[pathStringVec.size()-1]=="msg")
        {
            filePathVec.push_back(replace(entry.path().string(),"\\","/"));
        }

        if (std::filesystem::is_directory(entry.path()))
        {
            TraverseFolder(entry.path().string());  
        }
    }
}
void GetStruct(std::string& folderPath)
{
    headerFileStr.append("#include <Serializer.hpp>\n");
    headerFileStr.append("#include <nlohmann/json.hpp>\n");
    headerFileStr.append("#include <vector>\n");
    headerFileStr.append("#include <string>\n");
    headerFileStr.append("#include <chrono>\n");
    headerFileStr.append("typedef int8_t int8; \n");
    headerFileStr.append("typedef int16_t int16; \n");
    headerFileStr.append("typedef int32_t int32; \n");
    headerFileStr.append("typedef int64_t int64; \n");
    headerFileStr.append("typedef uint8_t uint8; \n");
    headerFileStr.append("typedef uint16_t uint16; \n");
    headerFileStr.append("typedef uint32_t uint32; \n");
    headerFileStr.append("typedef uint64_t uint64; \n");
    headerFileStr.append("typedef std::string string; \n");
    headerFileStr.append("typedef double float64; \n");
    headerFileStr.append("typedef float float32; \n");
    headerFileStr.append("typedef char byte; \n");
    headerFileStr.append("typedef long long time; \n");
    headerFileStr.append("typedef long long duration; \n");

    headerFileStr.append("typedef struct\n");
    headerFileStr.append("{\n");
    headerFileStr.append("    float32 r; \n");
    headerFileStr.append("    float32 g; \n");
    headerFileStr.append("    float32 b; \n");
    headerFileStr.append("    float32 a; \n");
    headerFileStr.append("    friend Serializer& operator>>(Serializer & in, ColorRGBA & f)\n");
    headerFileStr.append("    {\n");
    headerFileStr.append("        in >> f.r >> f.g >> f.b >> f.a; \n");
    headerFileStr.append("        return in; \n");
    headerFileStr.append("    }\n");
    headerFileStr.append("    friend Serializer& operator<<(Serializer & out, ColorRGBA & f)\n");
    headerFileStr.append("    {\n");
    headerFileStr.append("        out << f.r << f.g << f.b << f.a; \n");
    headerFileStr.append("        return out; \n");
    headerFileStr.append("    }\n");
    headerFileStr.append("    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ColorRGBA, r, g, b, a);\n");
    headerFileStr.append("}ColorRGBA; \n");
    for (const std::string& path : filePathVec)
    {
        std::vector<std::string> structNameVec = split(path, '/');
        if (structNameVec.size() != 0)
        {
            std::string structName = replace(structNameVec[structNameVec.size() - 1], ".msg", "");
            headerFileStr.append("struct " + structName + ";\n");
            headerFileStr.append("    void to_json(nlohmann::json & nlohmann_json_j, const " + structName + " & nlohmann_json_t);\n");
            headerFileStr.append("    void from_json(const nlohmann::json & nlohmann_json_j, " + structName + " & nlohmann_json_t);\n");
            typeVec.push_back(structName);
        }                

    }        
    headerFileStr.append("\n");
    for (const std::string& path : filePathVec)
    {
        std::fstream msgFile(path);
        std::string varLine;

        std::vector<std::string> structNameVec = split(path, '/');

        std::string structName = replace(structNameVec[structNameVec.size() - 1], ".msg", "");

        headerFileStr.append("struct " + structName + "\n{\n");
        std::vector<std::string> varNameVec;
        std::vector<std::string> varTypeVec;
        while (std::getline(msgFile, varLine))
        {        
            //std::cout << varLine << std::endl;
       
            int i_ = 0;
            while (varLine[i_] == ' ')
            {
                i_++;
            }
            varLine= varLine.substr(i_,std::string::npos);           
            size_t pos = varLine.find('#');    
            varLine = varLine.substr(0, pos);
            std::string varName;
            std::string varType;
            if (varLine.size()!=0&& !isAllSpaces(varLine))
            {
                std::vector<std::string> tempLineVec= split(varLine, ' ');
                varName = tempLineVec[1];
                int i_ = 1;
                while (varName==""&&i_!= tempLineVec.size())
                {
                    i_++;
                    varName = tempLineVec[i_];

                }

                std::vector<std::string> tempTypeVec = split(tempLineVec[0], '/');

                if (tempLineVec[0].find('/') == std::string::npos)
                {
                    varType= tempLineVec[0];
                }
                else
                {
                    varType = tempTypeVec[tempTypeVec.size() - 1];
                }                
                if (varType != "Header")
                {
                    int typeGet = 0;                        
                    std::string trueName;
                    for (const std::string& typeName_ : typeVec)
                    {
                        std::vector<std::string> tempTypeVec_ = split(varType, '[');
                        if (varType.find('[')== std::string::npos)
                        {
                            trueName= varType;

                        }
                        else
                        {                   

                            trueName = tempTypeVec_[0];
                        }
                        if (typeName_ == trueName)
                        {
                            typeGet = 1;
                        }
                    }                    
                    std::vector<std::string> tempTypeVec__ = split(varType, '[');
                    if (varType.find('[')== std::string::npos)
                    {
                        
                        headerFileStr.append("    "+varType + " " + replace(varName, " ", "") + ";\n");
 
                    }
                    else
                    {
                        if (tempTypeVec__[1]=="]")
                        {
                            headerFileStr.append("    std::vector<" + trueName + "> " + replace(varName, " ", "") + ";\n");
                        }
                        else
                        {
                            headerFileStr.append("    " + trueName + " " + replace(varName, " ", "") +"["+ tempTypeVec__ [1] + ";\n");
                        }
                    } 
                    if (varName.find('=')!=std::string::npos)
                    {
                        std::vector<std::string> tempNameVec__ = split(varName, '=');
                        varName=replace(tempNameVec__[0], " ", "");
                    }
                    varNameVec.push_back(varName);
                    varTypeVec.push_back(varType);
                    if (typeGet==1)
                    {

                    
                    }
                    else
                    {
                        std::cout << "UnKnow type "<< varType << std::endl;
                    }
                }

            }

        }
        headerFileStr.append("    friend Serializer& operator>>(Serializer & in, "+ structName+" & f)\n");
        headerFileStr.append("    {\n");
        headerFileStr.append("        in");
        for (std::vector<std::string>::iterator it= varNameVec.begin(); it != varNameVec.end(); it++)
        {
            headerFileStr.append(" >> f."+(*it));
        }
        headerFileStr.append("; \n");
        headerFileStr.append("        return in; \n");
        headerFileStr.append("    }\n");
        headerFileStr.append("    friend Serializer& operator<<(Serializer & out, " + structName + " & f)\n");
        headerFileStr.append("    {\n");
        headerFileStr.append("        out");
        for (std::vector<std::string>::iterator it = varNameVec.begin(); it != varNameVec.end(); it++)
        {
            headerFileStr.append(" << f." + (*it));
        }
        headerFileStr.append("; \n");
        headerFileStr.append("        return out; \n");
        headerFileStr.append("    }\n");



        headerFileStr.append("    void to_json(nlohmann::json & nlohmann_json_j, const " + structName + " & nlohmann_json_t)\n");
        headerFileStr.append("    {\n");
        
        for (int it = 0; it != varNameVec.size(); it++)
        {
            std::vector<std::string> tempTypeVec__ = split(varTypeVec[it], '[');
            if (varTypeVec[it].find('[') == std::string::npos)
            {
                headerFileStr.append("        nlohmann_json_j[\""+ varNameVec[it] +"\"] = nlohmann_json_t."+ varNameVec[it] +";\n");
            }
            else
            {
                if (tempTypeVec__[1] == "]")
                {
                    headerFileStr.append("        for(int "+ varNameVec[it] +"_it = 0;" + varNameVec[it] + "_it !=nlohmann_json_t."+ varNameVec[it]+".size();"+ varNameVec[it] +"_it++)\n");
                    headerFileStr.append("        {\n");
                    headerFileStr.append("            nlohmann_json_j[\"" + varNameVec[it] + "\"][std::to_string("+ varNameVec[it]+"_it" +")] = nlohmann_json_t." + varNameVec[it] + "["+ varNameVec[it] + "_it"+"];\n");
                    headerFileStr.append("        }\n");
                }
                else
                {
                    headerFileStr.append("        for(int " + varNameVec[it] + "_it = 0;" + varNameVec[it] + "_it !=" + replace(tempTypeVec__[1],"]","") + ";" + varNameVec[it] + "_it++)\n");
                    headerFileStr.append("        {\n");
                    headerFileStr.append("            nlohmann_json_j[\"" + varNameVec[it] + "\"][std::to_string(" + varNameVec[it] + "_it" + ")] = nlohmann_json_t." + varNameVec[it] + "[" + varNameVec[it] + "_it" + "];\n");
                    headerFileStr.append("        }\n");
                    //headerFileStr.append("    " + trueName + " " + varName + "[" + tempTypeVec__[1] + ";\n");
                }
            }

        }

        headerFileStr.append("    }\n");

        headerFileStr.append("    void from_json(const nlohmann::json & nlohmann_json_j, " + structName + " & nlohmann_json_t)\n");
        headerFileStr.append("    {\n");

        for (int it = 0; it != varNameVec.size(); it++)
        {
            std::vector<std::string> tempTypeVec__ = split(varTypeVec[it], '[');
            if (varTypeVec[it].find('[') == std::string::npos)
            {
                headerFileStr.append("        nlohmann_json_j.at(\"" + varNameVec[it] + "\").get_to(nlohmann_json_t." + varNameVec[it] + ");\n");
            }
            else
            {
                if (tempTypeVec__[1] == "]")
                {
                    headerFileStr.append("        nlohmann_json_t." + varNameVec[it]+".clear();\n");
                    headerFileStr.append("        for(int " + varNameVec[it] + "_it = 0;" + varNameVec[it] + "_it !=nlohmann_json_t." + varNameVec[it] + ".size();" + varNameVec[it] + "_it++)\n");
                    headerFileStr.append("        {\n");
                    headerFileStr.append("            "+ tempTypeVec__[0]+" "+varNameVec[it]+"_temp;\n");
                    headerFileStr.append("            nlohmann_json_j.at(\"" + varNameVec[it] + "\").at(std::to_string(" + varNameVec[it] + "_it" + ")).get_to(" +varNameVec[it] + "_temp" ");\n");
                    headerFileStr.append("            nlohmann_json_t." + varNameVec[it] + ".push_back("+ varNameVec[it] + "_temp"+");\n");
                    headerFileStr.append("        }\n");
                }
                else
                {
                    headerFileStr.append("        for(int " + varNameVec[it] + "_it = 0;" + varNameVec[it] + "_it !=" + replace(tempTypeVec__[1], "]", "") + ";" + varNameVec[it] + "_it++)\n");
                    headerFileStr.append("        {\n");
                    headerFileStr.append("            nlohmann_json_j.at(\"" + varNameVec[it] + "\").at(std::to_string(" + varNameVec[it] + "_it" + ")).get_to(" + varNameVec[it] + "[" + varNameVec[it] + "_it" + "]);\n");
                    headerFileStr.append("        }\n");
                    //headerFileStr.append("    " + trueName + " " + varName + "[" + tempTypeVec__[1] + ";\n");
                }
            }

        }

        headerFileStr.append("    }\n");

        headerFileStr.append("};\n");

    }
    std::cout << headerFileStr << std::endl;
    std::fstream outFile;
    outFile.open(folderPath+"/MsgHeader.hpp",std::ios::out);
    outFile << headerFileStr << std::endl;
    outFile.close();
}

int main(int nArgc, char* argv[])
{
    if (nArgc != 2)
    {
        std::cout << "please input file path\n";
        return 0;
    }
    std::string folderPath = argv[1];
    TraverseFolder(folderPath);
    GetStruct(folderPath);
    return 0;
}
