#ifndef ConfigReader_h
#define ConfigReader_h

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


class ConfigReader {
public:
  static json LoadConfig(const std::string& filename);
  static double GetNormFactor(const json& config, const std::string& sampleName);
};

#endif