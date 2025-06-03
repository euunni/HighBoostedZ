#ifndef NtupleReader_h
#define NtupleReader_h 1

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "TFile.h"
#include "TROOT.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TChain.h"


class NtupleReader
{
public:
  NtupleReader() :
    fChain(new TChain("Events")),
    fReader(nullptr),
    fNFiles(0),
    fGenWeight(nullptr)
  {
  }
  ~NtupleReader() {
    delete fChain;
  }

  void Init(const std::string& sampleName, const std::string& era, const int& idx = 0, const int& filesPerJob = 10);
  // bool Next() { return fReader->Next(); }
  TChain* GetChain() { return fChain; }
  TTreeReader* GetReader() { return fReader; }
  void SetMC();
  TTreeReaderValue<float>* GetGenWeight();
  std::string GetSample() const;

private:
  TChain* fChain;
  TTreeReader* fReader;  
  int fNFiles;
  TTreeReaderValue<float>* fGenWeight;

  bool GetFile(const std::string& sampleName, const std::string& era, const int& idx, const int& filesPerJob);
};

#endif
