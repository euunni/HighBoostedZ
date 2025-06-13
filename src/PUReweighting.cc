#include "PUReweighting.h"
#include <TFile.h>
#include <TH1D.h>
#include <iostream>
#include <TROOT.h>

PUReweighting::PUReweighting(const std::string& dataPUFile, const std::string& mcPUFile) {
  CalcWeight(dataPUFile, mcPUFile);
}

double PUReweighting::GetWeight(int puValue) const {
  return fPU[puValue];
}

void PUReweighting::CalcWeight(const std::string& dataPUFile, const std::string& mcPUFile) {
  TFile dataFile(dataPUFile.c_str(), "READ");
  TFile mcFile(mcPUFile.c_str(), "READ");

  if (!dataFile.IsOpen() || !mcFile.IsOpen()) {
    std::cerr << "Error opening data or MC file for PU reweighting." << std::endl;
    return;
  }

  TH1D* hDataOrig = dynamic_cast<TH1D*>(dataFile.Get("pileup"));
  TH1D* hMCOrig = dynamic_cast<TH1D*>(mcFile.Get("pileup"));

  gROOT->cd();
  fHData.reset((TH1D*)hDataOrig->Clone("hDataClone"));
  fHMC.reset((TH1D*)hMCOrig->Clone("hMCClone"));

  fHData->Scale(1.0 / fHData->Integral());
  fHMC->Scale(1.0 / fHMC->Integral());

  int nbins = fHData->GetNbinsX();
  fPU.resize(nbins);

  for (int i = 0; i < nbins; i++) {
    double dataValue = fHData->GetBinContent(i+1);
    double mcValue = fHMC->GetBinContent(i+1);
    fPU[i] = (mcValue > 0) ? dataValue / mcValue : 1.0;
  }

  dataFile.Close();
  mcFile.Close();
}

TH1D* PUReweighting::GetMCWeightedHist() {
  if (!fHMC) return nullptr;
  if (!fHMCReweighted) {
    gROOT->cd();
    fHMCReweighted.reset((TH1D*)fHMC->Clone("hMCReweighted"));
    fHMCReweighted->SetTitle("PU MC Reweighted");
    fHMCReweighted->Reset();

    int nbins = fHMC->GetNbinsX();
    for (int i = 1; i <= nbins; i++) {
      int puValue = i - 1;
      double puProb = fHMC->GetBinContent(i);
      double weight = (puProb > 0) ? GetWeight(puValue) : 1.0;
      fHMCReweighted->SetBinContent(i, puProb * weight);
    }
  }
  return fHMCReweighted.get();
}
