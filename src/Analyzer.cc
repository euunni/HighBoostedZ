#include "Analyzer.h"
#include "NtupleReader.h"
#include "Muon.h"

#include <iostream>
#include <math.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TMath.h>
#include <cstdlib>
#include <fstream>
#include <TChainElement.h>


bool Analyzer::Init(const std::string& sampleName, const std::string& era, const int& idx)
{
  fSampleName = sampleName;
  fEra = era;
  fIdx = idx;
  
  std::string configPath = "../input/config/" + era + "/config.json";
  fMuonConfig = Selection::Load(configPath);
  
  int filesPerJob = 10; // default
  if (fMuonConfig.j.contains("Processing") && fMuonConfig.j["Processing"].contains("FilesPerJob")) {
    filesPerJob = fMuonConfig.j["Processing"]["FilesPerJob"].get<int>();
  }
  
  fNtupleReader = std::make_unique<NtupleReader>();
  fNtupleReader->Init(sampleName, era, idx, filesPerJob);
  fReader = fNtupleReader->GetReader(); 
  fMuon->Init(fReader);
  
  system(("mkdir -p ../output/250526/" + era + "/" + sampleName).c_str());
  fOutputName = "../output/250526/" + era + "/" + sampleName + "/" + sampleName + "_" + std::to_string(idx) + ".root";

  fIsMC = fMuonConfig.j["IsMC"].contains(sampleName) ? 
          fMuonConfig.j["IsMC"][sampleName].get<bool>() : true;  
          
  if (fIsMC) {
    fNtupleReader->SetMC();
  }

  SetHist();
  
  return true;
}

void Analyzer::Run()
{  
  TChain* chain = fNtupleReader->GetChain();
  int nEntries = chain->GetEntries();

  std::string sample = fNtupleReader->GetSample();
  std::cout << "Processing " << fSampleName << " (" << fEra << ") with " << nEntries << " events" << std::endl;

  double totalWeight = 0.;
  bool isNNLO = (sample.find("NNLO") != std::string::npos);


  // Event loop
  for (int entry = 0; entry < nEntries; ++entry) {
    fReader->SetEntry(entry);
    
    if (entry % 10000 == 0) {
      std::cout << "Processing event " << entry << "/" << nEntries << std::endl;
    }

    double evtWeight = 1.;

    if (fIsMC) {
      evtWeight = **(fNtupleReader->GetGenWeight());
      if (isNNLO) {
        evtWeight = (evtWeight > 0) ? 1. : -1.;
      }
    }

    totalWeight += evtWeight;
    
    // Trigger selection
    std::vector<std::string> triggerList = fMuon->GetTriggers(fMuonConfig, fSampleName);
    if (!(fMuon->PassTriggers(triggerList))) {
      continue;
    }

    // Find dimuons
    auto dimuon = fMuon->GetDimuon(fMuonConfig);

    if (dimuon.isValid) {
      h_SingleMuonPt->Fill(dimuon.leading->Pt(), evtWeight);
      h_SingleMuonEta->Fill(dimuon.leading->Eta(), evtWeight);
      h_SingleMuonPhi->Fill(dimuon.leading->Phi(), evtWeight);
      h_SingleMuonPt->Fill(dimuon.subleading->Pt(), evtWeight);
      h_SingleMuonEta->Fill(dimuon.subleading->Eta(), evtWeight);
      h_SingleMuonPhi->Fill(dimuon.subleading->Phi(), evtWeight);

      h_LeadingMuonPt->Fill(dimuon.leading->Pt(), evtWeight);
      h_LeadingMuonEta->Fill(dimuon.leading->Eta(), evtWeight);
      h_LeadingMuonPhi->Fill(dimuon.leading->Phi(), evtWeight);

      h_SubleadingMuonPt->Fill(dimuon.subleading->Pt(), evtWeight);
      h_SubleadingMuonEta->Fill(dimuon.subleading->Eta(), evtWeight);
      h_SubleadingMuonPhi->Fill(dimuon.subleading->Phi(), evtWeight);

      h_DimuonPt->Fill(dimuon.dimuon.Pt(), evtWeight);
      h_DimuonRapidity->Fill(dimuon.dimuon.Rapidity(), evtWeight);
      h_DimuonPhi->Fill(dimuon.dimuon.Phi(), evtWeight);
      h_DimuonMass->Fill(dimuon.dimuon.M(), evtWeight);
    }
  } // End of event loop

  h_TotalWeight->SetBinContent(1, totalWeight);
}

void Analyzer::End()
{
  WriteHist();
}

void Analyzer::SetHist()
{
  h_TotalWeight = new TH1F("h_total_weight", "Total weight;Bin;Total weight", 1, 0, 1);

  h_SingleMuonPt = new TH1F("h_singlemuon_pt", "Single muon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_SingleMuonEta = new TH1F("h_singlemuon_eta", "Single muon #eta after z mass cut;#eta;Events", 60, -3, 3);
  h_SingleMuonPhi = new TH1F("h_singlemuon_phi", "Single muon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);

  h_LeadingMuonPt = new TH1F("h_leadingmuon_pt", "Leading muon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_LeadingMuonEta = new TH1F("h_leadingmuon_eta", "Leading muon #eta after z mass cut;#eta;Events", 60, -3, 3);
  h_LeadingMuonPhi = new TH1F("h_leadingmuon_phi", "Leading muon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);

  h_SubleadingMuonPt = new TH1F("h_subleadingmuon_pt", "Subleading muon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_SubleadingMuonEta = new TH1F("h_subleadingmuon_eta", "Subleading muon #eta after z mass cut;#eta;Events", 60, -3, 3);
  h_SubleadingMuonPhi = new TH1F("h_subleadingmuon_phi", "Subleading muon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);
  
  h_DimuonPt = new TH1F("h_dimuon_pt", "Dimuon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_DimuonRapidity = new TH1F("h_dimuon_rapidity", "Dimuon rapidity after z mass cut;y;Events", 60, -3, 3);
  h_DimuonPhi = new TH1F("h_dimuon_phi", "Dimuon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);
  h_DimuonMass = new TH1F("h_dimuon_mass", "Dimuon mass after z mass cut;m [GeV];Events", 10000, 0, 10000);
}

void Analyzer::FillHist()
{
  // will add later.
}

void Analyzer::WriteHist()
{
  TFile outputFile(fOutputName.c_str(), "RECREATE");

  h_TotalWeight->Write();

  h_SingleMuonPt->Write();
  h_SingleMuonEta->Write();
  h_SingleMuonPhi->Write();

  h_LeadingMuonPt->Write();
  h_LeadingMuonEta->Write();
  h_LeadingMuonPhi->Write();

  h_SubleadingMuonPt->Write();
  h_SubleadingMuonEta->Write();
  h_SubleadingMuonPhi->Write();

  h_DimuonPt->Write();
  h_DimuonRapidity->Write();
  h_DimuonPhi->Write();
  h_DimuonMass->Write();

  outputFile.Close();
  
  std::cout << "Output saved to: " << fOutputName << std::endl;
}
