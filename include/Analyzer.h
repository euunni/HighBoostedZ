#ifndef Analyzer_h
#define Analyzer_h 1

#include "NtupleReader.h"
#include "Muon.h"

#include <string>
#include <vector>
#include <memory>
#include "TH1.h"
#include "TH2.h"


class Analyzer
{
public:
  Analyzer() :
    fMuon(new Muon())
  {
  }
  ~Analyzer() {
    delete fMuon;
  }

  bool Init(const std::string& sampleName, const std::string& era, const int& idx);
  void Run();
  void End();
  bool IsMC() { return fIsMC; }

private:
  std::unique_ptr<NtupleReader> fNtupleReader;
  TTreeReader* fReader;
  Muon* fMuon;

  std::string fSampleName;
  std::string fEra;
  std::string fOutputName;
  int fIdx;
  Selection fMuonConfig;
  bool fIsMC;

  TH1F* h_TotalWeight;
  
  TH1F* h_MuonPt;
  TH1F* h_MuonEta;
  TH1F* h_MuonPhi;

  TH1F* h_MuonPt_afterTrigger;
  TH1F* h_MuonEta_afterTrigger;
  TH1F* h_MuonPhi_afterTrigger;

  TH1F* h_MuonPt_selected;
  TH1F* h_MuonEta_selected;
  TH1F* h_MuonPhi_selected;

  TH1F* h_MuonPt_afterPt;
  TH1F* h_MuonEta_afterPt;
  TH1F* h_MuonPhi_afterPt;

  TH1F* h_MuonPt_afterPtEta;
  TH1F* h_MuonEta_afterPtEta;
  TH1F* h_MuonPhi_afterPtEta;

  TH1F* h_MuonPt_afterPtEtaId;
  TH1F* h_MuonEta_afterPtEtaId;
  TH1F* h_MuonPhi_afterPtEtaId;

  TH1F* h_MuonPt_afterPtEtaIdTkIso;
  TH1F* h_MuonEta_afterPtEtaIdTkIso;
  TH1F* h_MuonPhi_afterPtEtaIdTkIso;

  TH1F* h_SingleMuonPt;
  TH1F* h_SingleMuonEta;
  TH1F* h_SingleMuonPhi;

  TH1F* h_LeadingMuonPt;
  TH1F* h_LeadingMuonEta;
  TH1F* h_LeadingMuonPhi;

  TH1F* h_SubleadingMuonPt;
  TH1F* h_SubleadingMuonEta;
  TH1F* h_SubleadingMuonPhi;

  TH1F* h_DimuonPt;
  TH1F* h_DimuonRapidity;
  TH1F* h_DimuonPhi;
  TH1F* h_DimuonMass;

  TH1F* h_DimuonPt_MassCut;
  TH1F* h_DimuonRapidity_MassCut;
  TH1F* h_DimuonPhi_MassCut;
  TH1F* h_DimuonMass_MassCut;
  
  void SetHist();
  void FillHist();
  void WriteHist();
};

#endif
