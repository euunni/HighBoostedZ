#include "NtupleReader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <unistd.h>


void NtupleReader::Init(const std::string& sampleName, const std::string& era)
{
  fSampleName = sampleName;
  fEra = era;
  
  GetFile(sampleName, era);

  fReader = new TTreeReader(fChain);
}

bool NtupleReader::GetFile(const std::string& sampleName, const std::string& era) {
    
  std::string inputDir = "../input/" + era + "/" + sampleName;
  fNFiles = 0;
  
  try {
    for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
      if (entry.is_regular_file()) {
        std::string filePath = entry.path().string();
        std::ifstream inputFile(filePath);

        std::string line;
        while (std::getline(inputFile, line)) {
          if (line.empty()) continue;

          size_t pos = line.find("/pnfs/");
          if (pos != std::string::npos) {
            line = line.substr(pos);
          }

          fChain->Add(line.c_str());
          fNFiles++;
        }
        
        inputFile.close();
      }
    }
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "Error: Could not access directory: " << inputDir << std::endl;
    return false;
  }
  std::cout << "Successfully added " << fNFiles << " list files from " << inputDir << std::endl;

  return true;
}

std::string NtupleReader::GetSample() const {
  TFile* file = fChain->GetCurrentFile();
  if (file) {
    return file->GetName();
  }
  return "";
}

void NtupleReader::SetMC() {
  genWeight = new TTreeReaderValue<float>(*fReader, "genWeight");
}

TTreeReaderValue<float>* NtupleReader::GetGenWeight() {
  return genWeight;
}