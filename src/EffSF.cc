#include "EffSF.h"
#include <TFile.h>
#include <TROOT.h>
#include "TH2.h"
#include <iostream>
#include <cmath>


EffSF::EffSF(const std::string& idFile,
              const std::string& isoFile,
              const std::string& triggerFile,
              const std::map<std::string, std::string>& histNames) {
  fID = LoadHist(idFile, histNames.at("ID_Data"), histNames.at("ID_MC"));
  fISO = LoadHist(isoFile, histNames.at("ISO_Data"), histNames.at("ISO_MC"));
  fTrigger = LoadHist(triggerFile, histNames.at("Trigger_Data"), histNames.at("Trigger_MC"));
}

double EffSF::GetIDSF(const TLorentzVector& muon) const {
  return GetSF(fID.hData.get(), fID.hMC.get(), muon.Pt(), std::abs(muon.Eta()));
}

double EffSF::GetISOSF(const TLorentzVector& muon) const {
  return GetSF(fISO.hData.get(), fISO.hMC.get(), muon.Pt(), std::abs(muon.Eta()));
}

double EffSF::GetTrigSF(const TLorentzVector& muon1, const TLorentzVector& muon2) const {
  if (!fTrigger.hData || !fTrigger.hMC) return 1.0;

  double eff1Data = GetEff(fTrigger.hData.get(), muon1.Pt(), std::abs(muon1.Eta()));
  double eff2Data = GetEff(fTrigger.hData.get(), muon2.Pt(), std::abs(muon2.Eta()));
  double eff1MC   = GetEff(fTrigger.hMC.get(), muon1.Pt(), std::abs(muon1.Eta()));
  double eff2MC   = GetEff(fTrigger.hMC.get(), muon2.Pt(), std::abs(muon2.Eta()));

  // Event-level trigger efficiency: 1 - (1 - eff1) * (1 - eff2)
  double eventEffData = 1.0 - (1.0 - eff1Data) * (1.0 - eff2Data);
  double eventEffMC   = 1.0 - (1.0 - eff1MC) * (1.0 - eff2MC);

  return (eventEffMC > 0) ? eventEffData / eventEffMC : 1.0;
}

double EffSF::GetSF(const TH2* hData, const TH2* hMC, double pt, double eta) {
  if (!hData || !hMC) return 1.0;

  double dataEff = GetEff(hData, pt, eta);
  double mcEff   = GetEff(hMC, pt, eta);

  return (dataEff > 0 && mcEff > 0) ? dataEff / mcEff : 1.0;
}

double EffSF::GetEff(const TH2* hist, double pt, double eta)
{
  if (!hist) return 1.0;

  int x = hist->GetXaxis()->FindBin(eta);
  int y = hist->GetYaxis()->FindBin(pt);

  // Clamp to histogram bounds
  if (x < 1) x = 1;
  if (x > hist->GetNbinsX()) x = hist->GetNbinsX();
  if (y < 1) y = 1;
  if (y > hist->GetNbinsY()) y = hist->GetNbinsY();

  return hist->GetBinContent(x, y);
}

EffSF::HistPair EffSF::LoadHist(const std::string& filename, const std::string& dataHistName, const std::string& mcHistName) {
  HistPair histPair;
  
  TFile file(filename.c_str(), "READ");
  if (!file.IsOpen()) return histPair;

  // Load data histogram
  TH2* hData = (TH2*)file.Get(dataHistName.c_str());
  if (hData) {
    gROOT->cd();
    histPair.hData.reset((TH2*)hData->Clone());
    histPair.hData->SetDirectory(nullptr);
  }

  // Load MC histogram  
  TH2* hMC = (TH2*)file.Get(mcHistName.c_str());
  if (hMC) {
    gROOT->cd();
    histPair.hMC.reset((TH2*)hMC->Clone());
    histPair.hMC->SetDirectory(nullptr);
  }

  return histPair;
} 
