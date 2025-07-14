#ifndef ConfigReader_h
#define ConfigReader_h

#include <string>
#include <memory>
#include <tuple>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct CorrSwitch {
  bool doRoch = true;
  bool doNorm = true;
  bool doPU = true;
  bool doL1Pre = true;
  bool doEffSF = true;
};

class ConfigReader {
public:
  static json LoadConfig(const std::string& filename);
  
  static double GetNormFactor(const json& config, const std::string& sampleName);
  static std::pair<std::string, std::string> ReadPU(const json& config);
  static CorrSwitch ReadCorrections(const json& config);
  static int ReadFilesPerJob(const json& config);
  static bool ReadIsMC(const json& config, const std::string& sampleName);
  static std::string ReadRoccoRPath(const json& config);
  static std::tuple<std::string, std::string, std::string, std::map<std::string, std::string>> ReadEffSF(const json& config);

};

#endif