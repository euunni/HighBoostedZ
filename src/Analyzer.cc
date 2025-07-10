#include "Analyzer.h"
#include "NtupleReader.h"
#include "Muon.h"
#include "RoccoR.h"

#include <iostream>
#include <math.h>
#include <TFile.h>
#include <TH1D.h>
#include <TMath.h>
#include <cstdlib>
#include <fstream>
#include <TChainElement.h>


bool Analyzer::Init(const std::string& sampleName, const std::string& era, const int& idx)
{
  fSampleName = sampleName;
  fEra = era;
  
  // Load config
  std::string configPath = "/u/user/haeun/CMSAnalysis/HighBoostedZ/Validation/HighBoostedZ/input/config/" + era + "/config.json";
  fConfig = Selection::Load(configPath);

  int filesPerJob = 10; // default
  if (fConfig.j.contains("Processing") && fConfig.j["Processing"].contains("FilesPerJob")) {
    filesPerJob = fConfig.j["Processing"]["FilesPerJob"].get<int>();
  }

  fIsMC = fConfig.j["IsMC"].contains(sampleName) ? 
          fConfig.j["IsMC"][sampleName].get<bool>() : true;  

  // Load correction switches from config
  if (fConfig.j.contains("Corrections")) {
    auto& cs = fConfig.j["Corrections"];
    if (cs.contains("RoccoR")) fCorr.doRoch = cs["RoccoR"].get<bool>();
    if (cs.contains("PU"))        fCorr.doPU   = cs["PU"].get<bool>();
    if (cs.contains("L1Prefire")) fCorr.doL1Pre= cs["L1Prefire"].get<bool>();
    if (cs.contains("Norm"))      fCorr.doNorm = cs["Norm"].get<bool>();
  }

  // Initialize ntuple reader
  fNtupleReader = std::make_unique<NtupleReader>();
  fNtupleReader->Init(sampleName, era, idx, filesPerJob);
  if (fIsMC) {
    fNtupleReader->SetMC();
  }
  fReader = fNtupleReader->GetReader(); 

  fMuon = std::make_unique<Muon>();
  fMuon->Init(fReader);

  // Initialize RoccoR (for Rochester muon momentum correction)
  std::string roccoRPath = fConfig.j["RoccoR"]["File"].get<std::string>();
  fRoccoR = std::make_unique<RoccoR>(roccoRPath);
  fMuon->SetRoccoR(fRoccoR.get(), fIsMC, fCorr.doRoch);

  // Get normalization factor
  // fNormFactor = ConfigReader::GetNormFactor(fConfig.j, fSampleName);

  // Initialize PU reweighting
  std::string dataPU = fConfig.j["PU"]["Data"].get<std::string>();
  std::string mcPU = fConfig.j["PU"]["MC"].get<std::string>();
  fPUReweighting = std::make_unique<PUReweighting>(dataPU, mcPU);

  // Set output file
  std::string baseDir = "/u/user/haeun/CMSAnalysis/HighBoostedZ/Validation/HighBoostedZ/output/250710_RoccoR_ChangeEvtWeight/root";
  system(("mkdir -p " + baseDir + "/" + era + "/" + sampleName).c_str());
  fOutputName = baseDir + "/" + era + "/" + sampleName + "/" + sampleName + "_" + std::to_string(idx) + ".root";

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
  // bool isNNLO = (sample.find("NNLO") != std::string::npos);


  // Event loop
  for (int entry = 0; entry < nEntries; ++entry) {
    fReader->SetEntry(entry);
    
    if (entry % 10000 == 0) {
      std::cout << "Processing event " << entry << "/" << nEntries << std::endl;
    }

    double evtWeight = 1.;
    double puWeight = 1.;
    double l1PreWeight = 1.;

    if (fIsMC) {
      evtWeight = **(fNtupleReader->GetGenWeight());
      // if (isNNLO) {
      //   evtWeight = (evtWeight > 0) ? 1. : -1.;
      // }
      evtWeight = (evtWeight > 0) ? 1. : -1.;

      puWeight = fPUReweighting->GetWeight(**(fNtupleReader->GetPU()));
      l1PreWeight = **(fNtupleReader->GetL1PreFiringWeight());
    }

    totalWeight += evtWeight;

    // Trigger selection
    std::vector<std::string> triggerList = fMuon->GetTriggers(fConfig, fSampleName);
    if (!(fMuon->PassTriggers(triggerList))) {
      continue;
    }

    // Find dimuons passing all selection criteria
    double weight = evtWeight;
    // if (fCorr.doNorm) weight *= fNormFactor;
    if (fCorr.doPU) weight *= puWeight;
    if (fCorr.doL1Pre) weight *= l1PreWeight;

    auto dimuon = fMuon->GetDimuon(fConfig);
    if (dimuon.isValid) {
      // Before correction
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

      // After correction
      h_SingleMuonPt_afterCorr->Fill(dimuon.leading->Pt(), weight);
      h_SingleMuonEta_afterCorr->Fill(dimuon.leading->Eta(), weight);
      h_SingleMuonPhi_afterCorr->Fill(dimuon.leading->Phi(), weight);
      h_SingleMuonPt_afterCorr->Fill(dimuon.subleading->Pt(), weight);
      h_SingleMuonEta_afterCorr->Fill(dimuon.subleading->Eta(), weight);
      h_SingleMuonPhi_afterCorr->Fill(dimuon.subleading->Phi(), weight);

      h_LeadingMuonPt_afterCorr->Fill(dimuon.leading->Pt(), weight);
      h_LeadingMuonEta_afterCorr->Fill(dimuon.leading->Eta(), weight);
      h_LeadingMuonPhi_afterCorr->Fill(dimuon.leading->Phi(), weight);

      h_SubleadingMuonPt_afterCorr->Fill(dimuon.subleading->Pt(), weight);
      h_SubleadingMuonEta_afterCorr->Fill(dimuon.subleading->Eta(), weight);
      h_SubleadingMuonPhi_afterCorr->Fill(dimuon.subleading->Phi(), weight);

      h_DimuonPt_afterCorr->Fill(dimuon.dimuon.Pt(), weight);
      h_DimuonRapidity_afterCorr->Fill(dimuon.dimuon.Rapidity(), weight);
      h_DimuonPhi_afterCorr->Fill(dimuon.dimuon.Phi(), weight);
      h_DimuonMass_afterCorr->Fill(dimuon.dimuon.M(), weight);
    }
  } // End of event loop

  h_TotalWeight->SetBinContent(1, totalWeight);
}

void Analyzer::End()
{
  WriteHist();

  // For PU Distribution
  // auto* hMCReweighted = fPUReweighting->GetMCWeightedHist();
  // TFile* out = new TFile("/u/user/haeun/CMSAnalysis/HighBoostedZ/Validation/HighBoostedZ/output/PUDistribution.root", "RECREATE");
  
  // fPUReweighting->GetDataHist()->Write("hData");
  // fPUReweighting->GetMCHist()->Write("hMC");
  // hMCReweighted->Write("hMCReweighted");
  
  // out->Close();
  // delete out;
}

void Analyzer::SetHist()
{
  h_TotalWeight = new TH1D("h_total_weight", "Total weight;Bin;Total weight", 1, 0, 1);

  h_SingleMuonPt = new TH1D("h_singlemuon_pt", "Single muon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_SingleMuonEta = new TH1D("h_singlemuon_eta", "Single muon #eta after z mass cut;#eta;Events", 60, -3, 3);
  h_SingleMuonPhi = new TH1D("h_singlemuon_phi", "Single muon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);

  h_LeadingMuonPt = new TH1D("h_leadingmuon_pt", "Leading muon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_LeadingMuonEta = new TH1D("h_leadingmuon_eta", "Leading muon #eta after z mass cut;#eta;Events", 60, -3, 3);
  h_LeadingMuonPhi = new TH1D("h_leadingmuon_phi", "Leading muon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);

  h_SubleadingMuonPt = new TH1D("h_subleadingmuon_pt", "Subleading muon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_SubleadingMuonEta = new TH1D("h_subleadingmuon_eta", "Subleading muon #eta after z mass cut;#eta;Events", 60, -3, 3);
  h_SubleadingMuonPhi = new TH1D("h_subleadingmuon_phi", "Subleading muon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);
  
  h_DimuonPt = new TH1D("h_dimuon_pt", "Dimuon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_DimuonRapidity = new TH1D("h_dimuon_rapidity", "Dimuon rapidity after z mass cut;y;Events", 60, -3, 3);
  h_DimuonPhi = new TH1D("h_dimuon_phi", "Dimuon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);
  h_DimuonMass = new TH1D("h_dimuon_mass", "Dimuon mass after z mass cut;m [GeV];Events", 10000, 0, 10000);

  h_SingleMuonPt_afterCorr = new TH1D("h_singlemuon_pt_after_corr", "Single muon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_SingleMuonEta_afterCorr = new TH1D("h_singlemuon_eta_after_corr", "Single muon #eta after z mass cut;#eta;Events", 60, -3, 3);
  h_SingleMuonPhi_afterCorr = new TH1D("h_singlemuon_phi_after_corr", "Single muon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);

  h_LeadingMuonPt_afterCorr = new TH1D("h_leadingmuon_pt_after_corr", "Leading muon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_LeadingMuonEta_afterCorr = new TH1D("h_leadingmuon_eta_after_corr", "Leading muon #eta after z mass cut;#eta;Events", 60, -3, 3);
  h_LeadingMuonPhi_afterCorr = new TH1D("h_leadingmuon_phi_after_corr", "Leading muon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);

  h_SubleadingMuonPt_afterCorr = new TH1D("h_subleadingmuon_pt_after_corr", "Subleading muon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_SubleadingMuonEta_afterCorr = new TH1D("h_subleadingmuon_eta_after_corr", "Subleading muon #eta after z mass cut;#eta;Events", 60, -3, 3);
  h_SubleadingMuonPhi_afterCorr = new TH1D("h_subleadingmuon_phi_after_corr", "Subleading muon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);
  
  h_DimuonPt_afterCorr = new TH1D("h_dimuon_pt_after_corr", "Dimuon pT after z mass cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_DimuonRapidity_afterCorr = new TH1D("h_dimuon_rapidity_after_corr", "Dimuon rapidity after z mass cut;y;Events", 60, -3, 3);
  h_DimuonPhi_afterCorr = new TH1D("h_dimuon_phi_after_corr", "Dimuon #phi after z mass cut;#phi;Events", 24, -M_PI, M_PI);
  h_DimuonMass_afterCorr = new TH1D("h_dimuon_mass_after_corr", "Dimuon mass after z mass cut;m [GeV];Events", 10000, 0, 10000);
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

  h_SingleMuonPt_afterCorr->Write();
  h_SingleMuonEta_afterCorr->Write();
  h_SingleMuonPhi_afterCorr->Write();

  h_LeadingMuonPt_afterCorr->Write();
  h_LeadingMuonEta_afterCorr->Write();
  h_LeadingMuonPhi_afterCorr->Write();

  h_SubleadingMuonPt_afterCorr->Write();
  h_SubleadingMuonEta_afterCorr->Write();
  h_SubleadingMuonPhi_afterCorr->Write();

  h_DimuonPt_afterCorr->Write();
  h_DimuonRapidity_afterCorr->Write();
  h_DimuonPhi_afterCorr->Write();
  h_DimuonMass_afterCorr->Write();

  outputFile.Close();
  
  std::cout << "Output saved to: " << fOutputName << std::endl;
}
