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
#include "Muon.h"


class NtupleReader
{
public:
  NtupleReader() :
    fChain(nullptr),
    fReader(nullptr)
  {
    fChain = new TChain("Events");
  }
  ~NtupleReader() {
    delete fChain;
  }

  void Init(const std::string& sampleName, const std::string& era);
  // bool Next() { return fReader->Next(); }

  TChain* GetChain() { return fChain; }
  TTreeReader* GetReader() { return fReader; }
  void SetMC();
  TTreeReaderValue<float>* GetGenWeight();
  std::string GetSample() const;

private:
  TChain* fChain;
  TTreeReader* fReader;  
  std::string fSampleName;
  std::string fEra;
  int fNFiles;
  TTreeReaderValue<float>* genWeight;

  bool GetFile(const std::string& sampleName, const std::string& era);
};

#endif
