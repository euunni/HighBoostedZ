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
  fID = LoadHist(idFile, histNames.at("ID"));
  fISO = LoadHist(isoFile, histNames.at("ISO"));
  fTriggerData = LoadHist(triggerFile, histNames.at("Trigger_Data"));
  fTriggerMC = LoadHist(triggerFile, histNames.at("Trigger_MC"));
}

double EffSF::GetIDSF(const TLorentzVector& muon) const {
  return GetEff(fID.get(), muon.Pt(), std::abs(muon.Eta()));
}

double EffSF::GetISOSF(const TLorentzVector& muon) const {
  return GetEff(fISO.get(), muon.Pt(), std::abs(muon.Eta()));
}

double EffSF::GetTrigSF(const TLorentzVector& muon1, const TLorentzVector& muon2) const {
  if (!fTriggerData || !fTriggerMC) return 1.0;

  double eff1Data = GetEff(fTriggerData.get(), muon1.Pt(), std::abs(muon1.Eta()));
  double eff2Data = GetEff(fTriggerData.get(), muon2.Pt(), std::abs(muon2.Eta()));
  double eff1MC   = GetEff(fTriggerMC.get(), muon1.Pt(), std::abs(muon1.Eta()));
  double eff2MC   = GetEff(fTriggerMC.get(), muon2.Pt(), std::abs(muon2.Eta()));

  // Event-level trigger efficiency: 1 - (1 - eff1) * (1 - eff2)
  double eventEffData = 1.0 - (1.0 - eff1Data) * (1.0 - eff2Data);
  double eventEffMC   = 1.0 - (1.0 - eff1MC) * (1.0 - eff2MC);

  return (eventEffMC > 0) ? eventEffData / eventEffMC : 1.0;
}

double EffSF::GetEff(const TH2* hist, double pt, double eta) {
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

std::unique_ptr<TH2> EffSF::LoadHist(const std::string& filename, const std::string& histName) {
  TFile file(filename.c_str(), "READ");
  if (!file.IsOpen()) return nullptr;
  
  TH2* h = (TH2*)file.Get(histName.c_str());
  if (!h) return nullptr;

  gROOT->cd();
  TH2* hClone = (TH2*)h->Clone();
  hClone->SetDirectory(nullptr);
  
  return std::unique_ptr<TH2>(hClone);
}
