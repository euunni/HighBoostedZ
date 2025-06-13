#ifndef Analyzer_h
#define Analyzer_h 1

#include "NtupleReader.h"
#include "Utils/ConfigReader.h"
#include "Muon.h"
#include "PUReweighting.h"

#include <string>
#include <vector>
#include <memory>
#include "TH1D.h"


class Analyzer
{
public:
  Analyzer() :
    fNtupleReader(nullptr),
    fMuon(nullptr),
    fSampleName(""),
    fEra(""),
    fIsMC(false),
    fOutputName(""),
    fNormFactor(1.)
  {
  }
  ~Analyzer() = default;

  bool Init(const std::string& sampleName, const std::string& era, const int& idx);
  void Run();
  void End();

private:
  std::unique_ptr<NtupleReader> fNtupleReader;
  std::unique_ptr<Muon> fMuon;
  TTreeReader* fReader;
  Selection fConfig;
  std::string fSampleName;
  std::string fEra;
  std::string fOutputName;
  bool fIsMC;
  double fNormFactor;
  std::unique_ptr<PUReweighting> fPUReweighting;
  
  TH1D* h_TotalWeight;

  TH1D* h_SingleMuonPt;
  TH1D* h_SingleMuonEta;
  TH1D* h_SingleMuonPhi;

  TH1D* h_LeadingMuonPt;
  TH1D* h_LeadingMuonEta;
  TH1D* h_LeadingMuonPhi;

  TH1D* h_SubleadingMuonPt;
  TH1D* h_SubleadingMuonEta;
  TH1D* h_SubleadingMuonPhi;

  TH1D* h_DimuonPt;
  TH1D* h_DimuonRapidity;
  TH1D* h_DimuonPhi;
  TH1D* h_DimuonMass;

  TH1D* h_SingleMuonPt_afterPU;
  TH1D* h_SingleMuonEta_afterPU;
  TH1D* h_SingleMuonPhi_afterPU;

  TH1D* h_LeadingMuonPt_afterPU;
  TH1D* h_LeadingMuonEta_afterPU;
  TH1D* h_LeadingMuonPhi_afterPU;

  TH1D* h_SubleadingMuonPt_afterPU;
  TH1D* h_SubleadingMuonEta_afterPU;
  TH1D* h_SubleadingMuonPhi_afterPU;

  TH1D* h_DimuonPt_afterPU;
  TH1D* h_DimuonRapidity_afterPU;
  TH1D* h_DimuonPhi_afterPU;
  TH1D* h_DimuonMass_afterPU;
  
  void SetHist();
  void FillHist();
  void WriteHist();
};

#endif
