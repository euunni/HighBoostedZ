#ifndef Analyzer_h
#define Analyzer_h 1

#include "NtupleReader.h"
#include "Utils/ConfigReader.h"
#include "Muon.h"

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

  TH1D* h_SingleMuonPt_afterNorm;
  TH1D* h_SingleMuonEta_afterNorm;
  TH1D* h_SingleMuonPhi_afterNorm;

  TH1D* h_LeadingMuonPt_afterNorm;
  TH1D* h_LeadingMuonEta_afterNorm;
  TH1D* h_LeadingMuonPhi_afterNorm;

  TH1D* h_SubleadingMuonPt_afterNorm;
  TH1D* h_SubleadingMuonEta_afterNorm;
  TH1D* h_SubleadingMuonPhi_afterNorm;

  TH1D* h_DimuonPt_afterNorm;
  TH1D* h_DimuonRapidity_afterNorm;
  TH1D* h_DimuonPhi_afterNorm;
  TH1D* h_DimuonMass_afterNorm;
  
  void SetHist();
  void FillHist();
  void WriteHist();
};

#endif
