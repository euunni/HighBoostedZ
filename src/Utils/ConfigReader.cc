#include "Utils/ConfigReader.h"
#include "EffSF.h"
#include <fstream>
#include <iostream>

json ConfigReader::LoadConfig(const std::string& filename) {
  json config;
  std::ifstream file(filename);
  file >> config;
  
  return config;
}

double ConfigReader::GetNormFactor(const json& config, const std::string& sampleName) {
  if (!config["Normalization"].contains(sampleName)) {
    std::cout << "Warning: Normalization factor not found for sample " << sampleName << std::endl;
    return 1.;
  }

  return config["Normalization"][sampleName].get<double>();
}

std::pair<std::string, std::string> ConfigReader::ReadPU(const json& config) {
  if (!config.contains("PU")) {
    std::cout << "Warning: PU section not found in config" << std::endl;
    return {"", ""};
  }
  auto& pu = config["PU"];
  return {pu["Data"].get<std::string>(), pu["MC"].get<std::string>()};
}

CorrSwitch ConfigReader::ReadCorrections(const json& config) {
  CorrSwitch corr;
  
  if (!config.contains("Corrections")) {
    std::cout << "Warning: Corrections section not found in config, using defaults" << std::endl;
    return corr;
  }
  
  auto& cs = config["Corrections"];
  if (cs.contains("RoccoR")) corr.doRoch = cs["RoccoR"].get<bool>();
  if (cs.contains("Norm")) corr.doNorm = cs["Norm"].get<bool>();
  if (cs.contains("PU")) corr.doPU = cs["PU"].get<bool>();
  if (cs.contains("L1Prefire")) corr.doL1Pre = cs["L1Prefire"].get<bool>();
  if (cs.contains("EffSF")) corr.doEffSF = cs["EffSF"].get<bool>();
  
  return corr;
}

int ConfigReader::ReadFilesPerJob(const json& config) {
  if (!config.contains("Processing")) {
    std::cout << "Warning: Processing section not found in config, using default (10)" << std::endl;
    return 10;
  }
  
  auto& processing = config["Processing"];
  if (!processing.contains("FilesPerJob")) {
    std::cout << "Warning: FilesPerJob not found in Processing section, using default (10)" << std::endl;
    return 10;
  }
  
  return processing["FilesPerJob"].get<int>();
}

bool ConfigReader::ReadIsMC(const json& config, const std::string& sampleName) {
  if (!config.contains("IsMC")) {
    std::cout << "Warning: IsMC section not found in config, assuming MC (true)" << std::endl;
    return true;
  }
  
  auto& isMC = config["IsMC"];
  if (!isMC.contains(sampleName)) {
    std::cout << "Warning: Sample " << sampleName << " not found in IsMC section, assuming MC (true)" << std::endl;
    return true;
  }
  
  return isMC[sampleName].get<bool>();
}

std::string ConfigReader::ReadRoccoRPath(const json& config) {
  if (!config.contains("RoccoR")) {
    std::cout << "Warning: RoccoR section not found in config" << std::endl;
    return "";
  }
  
  auto& roccoR = config["RoccoR"];
  if (!roccoR.contains("File")) {
    std::cout << "Warning: RoccoR File not found in config" << std::endl;
    return "";
  }
  
  return roccoR["File"].get<std::string>();
}

std::tuple<std::string, std::string, std::string, std::map<std::string, std::string>> ConfigReader::ReadEffSF(const json& config) {
  if (!config.contains("EffSF")) {
    std::cout << "Warning: EffSF section not found in config" << std::endl;
    return {"", "", "", {}};
  }
  
  auto& effSF = config["EffSF"];
  
  std::map<std::string, std::string> histNames = {
    {"ID", effSF["ID"]["Hist"].get<std::string>()},
    {"ISO", effSF["ISO"]["Hist"].get<std::string>()},
    {"Trigger_Data", effSF["Trigger"]["HistData"].get<std::string>()},
    {"Trigger_MC", effSF["Trigger"]["HistMC"].get<std::string>()}
  };

  return {
    effSF["ID"]["File"].get<std::string>(),
    effSF["ISO"]["File"].get<std::string>(),
    effSF["Trigger"]["File"].get<std::string>(),
    histNames
  };
}
