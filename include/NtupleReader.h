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
    genWeight(nullptr),
    Pileup_nTrueInt(nullptr)
  {
  }
  ~NtupleReader() {
    delete fChain;
  }

  void Init(const std::string& sampleName, const std::string& era, const int& idx = 0, const int& filesPerJob = 10);
  // bool Next() { return fReader->Next(); }
  TChain* GetChain() { return fChain; }
  TTreeReader* GetReader() { return fReader; }
  std::string GetSample() const;
  void SetMC();
  TTreeReaderValue<float>* GetGenWeight() const { return genWeight; }
  TTreeReaderValue<float>* GetPU() const { return Pileup_nTrueInt; }

private:
  TChain* fChain;
  TTreeReader* fReader;  
  int fNFiles;
  
  TTreeReaderValue<float>* genWeight;
  TTreeReaderValue<float>* Pileup_nTrueInt;

  bool GetFile(const std::string& sampleName, const std::string& era, const int& idx, const int& filesPerJob);
};

#endif
