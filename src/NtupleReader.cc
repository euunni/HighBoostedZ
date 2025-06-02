#include "NtupleReader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <unistd.h>
#include <algorithm>


void NtupleReader::Init(const std::string& sampleName, const std::string& era, const int& idx, const int& filesPerJob)
{
  fSampleName = sampleName;
  fEra = era;
  fFilesPerJob = filesPerJob;
  
  GetFile(sampleName, era, idx, filesPerJob);

  fReader = new TTreeReader(fChain);
}

bool NtupleReader::GetFile(const std::string& sampleName, const std::string& era, const int& idx, const int& filesPerJob) {
    
  std::string inputDir = "../input/" + era + "/" + sampleName;
  fNFiles = 0;
  
  try {
    std::vector<std::string> allFiles;
    
    for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
      if (entry.is_regular_file()) {
        std::string filePath = entry.path().string();
        std::ifstream inputFile(filePath);

        std::string line;
        while (std::getline(inputFile, line)) {
          if (line.empty()) continue;
          allFiles.push_back(line);
        }
        
        inputFile.close();
      }
    }
    
    std::sort(allFiles.begin(), allFiles.end());
    
    int startIdx = idx * filesPerJob;
    int endIdx = std::min(startIdx + filesPerJob, static_cast<int>(allFiles.size()));
    
    std::cout << "Job " << idx << " will process files from index " << startIdx 
              << " to " << (endIdx - 1) << " (total files: " << allFiles.size() << ")" << std::endl;
    
    for (int i = startIdx; i < endIdx; i++) {
      if (i < allFiles.size()) {
        std::string line = allFiles[i];
        
        size_t pos = line.find("/pnfs/");
        if (pos != std::string::npos) {
          line = line.substr(pos);
        }

        fChain->Add(line.c_str());
        fNFiles++;
      }
    }
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "Error: Could not access directory: " << inputDir << std::endl;
    return false;
  }
  
  std::cout << "Successfully added " << fNFiles << " ROOT files for job " << idx 
            << " (filesPerJob: " << filesPerJob << ")" << std::endl;

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