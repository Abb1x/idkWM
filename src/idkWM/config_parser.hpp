#ifndef CONFIG_H
#define CONFIG_H
#include "../lib/json.h"
#include "idkWM.hpp"
#include <vector>
namespace idkWM
{
#define JSON_GET(x) x.substr(1, x.size() - 2);

class ConfigParser
{
public:
    std::vector<std::string> get_json_array(std::string key);
    std::string get_string(std::string key);
    std::vector<std::string> split(std::string str, std::string delim);
    static ConfigParser *get();
};

} // namespace idkWM

#endif
