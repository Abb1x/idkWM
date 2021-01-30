#include "config_parser.hpp"
#include "log.hpp"
namespace idkWM
{
ConfigParser config_parser;

ConfigParser *ConfigParser::get()
{
    return &config_parser;
}
std::vector<std::string> split(std::string str, std::string delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == std::string::npos)
            pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty())
            tokens.push_back(token);
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

std::vector<std::string> ConfigParser::get_json_array(std::string key)
{
    // Parse the config
    std::string line;
    std::string final;

    std::ifstream config_file((std::string)getenv("HOME") + "/.config/idkWM/config.json");

    if (config_file.is_open())
    {
        while (getline(config_file, line))
        {
            final += line;
        }
        config_file.close();
    }
    log(final.c_str());
    json::jobject result = json::jobject::parse(final);

    std::string json_key = JSON_GET(result.get(key));

    std::vector<std::string> array = split(json_key, ",");

    for (size_t i = 0; i < array.size(); i++)
    {
        array[i] = array[i].substr(1, array[i].size() - 2);
    }

    return array;
}

std::string ConfigParser::get_string(std::string key)
{

    std::string line;
    std::string final;
    std::ifstream config_file((std::string)getenv("HOME") + "/.config/idkWM/config.json");

    if (config_file.is_open())
    {
        while (getline(config_file, line))
        {
            final += line;
        }
        config_file.close();
    }
    json::jobject result = json::jobject::parse(final);

    return JSON_GET(result.get(key));
}
} // namespace idkWM
