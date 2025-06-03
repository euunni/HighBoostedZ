#include "Utils/ConfigReader.h"
#include <fstream>

json ConfigReader::LoadConfig(const std::string& filename) {
  json config;
  std::ifstream file(filename);
  file >> config;
  
  return config;
}

double ConfigReader::GetNormFactor(const json& config, const std::string& sampleName) {
  if (config["Normalization"].contains(sampleName)) {
    return config["Normalization"][sampleName].get<double>();
  }

  return 1.;
}