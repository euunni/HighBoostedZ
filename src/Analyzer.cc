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
  
  // Initialize NtupleReader
  fNtupleReader = std::make_unique<NtupleReader>();
  fNtupleReader->Init(sampleName, era);
  fReader = fNtupleReader->GetReader(); 
  fMuon->Init(fReader);
  
  system(("mkdir -p ../output/250517/" + era + "/" + sampleName).c_str());
  fOutputName = "../output/250517/" + era + "/" + sampleName + "/" + sampleName + "_" + std::to_string(idx) + ".root";

  // Load config
  std::string configPath = "../input/config/" + era + "/config.json";
  fMuonConfig = Selection::Load(configPath);

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
  bool isNNLO = (sample.find("NNLO") != std::string::npos);
  
  std::cout << "Processing " << fSampleName << " (" << fEra << ") with " << nEntries << " events" << std::endl;

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
    
    // Reco muons
    auto fMuon4Vec = fMuon->Get4Vec();
    auto fMuonCharge = fMuon->GetCharge();
    
    for (int i = 0; i < fMuon4Vec.size(); i++) {
      h_MuonPt->Fill(fMuon4Vec.at(i).Pt(), evtWeight);
      h_MuonEta->Fill(fMuon4Vec.at(i).Eta(), evtWeight);
      h_MuonPhi->Fill(fMuon4Vec.at(i).Phi(), evtWeight);
    }

    // Trigger selection
    std::vector<std::string> triggerList = fMuon->GetTriggers(fMuonConfig, fSampleName);
    if (!(fMuon->PassTriggers(triggerList))) {
      continue;
    }

    for (int i = 0; i < fMuon4Vec.size(); i++) {
      h_MuonPt_afterTrigger->Fill(fMuon4Vec.at(i).Pt(), evtWeight);
      h_MuonEta_afterTrigger->Fill(fMuon4Vec.at(i).Eta(), evtWeight);
      h_MuonPhi_afterTrigger->Fill(fMuon4Vec.at(i).Phi(), evtWeight);
    }

    // Kinematic cuts
    SelectionOptions options;

    // Pt
    options.applyPtCut = true;
    auto muons_afterPt = fMuon->GetSelectedMuons(fMuonConfig, options);

    for (int i = 0; i < muons_afterPt.size(); i++) {
      h_MuonPt_afterPt->Fill(muons_afterPt.at(i).second.Pt(), evtWeight);
      h_MuonEta_afterPt->Fill(muons_afterPt.at(i).second.Eta(), evtWeight);
      h_MuonPhi_afterPt->Fill(muons_afterPt.at(i).second.Phi(), evtWeight);
    }

    // Pt + Eta
    options.applyEtaCut = true;
    auto muons_afterPtEta = fMuon->GetSelectedMuons(fMuonConfig, options);

    for (int i = 0; i < muons_afterPtEta.size(); i++) {
      h_MuonPt_afterPtEta->Fill(muons_afterPtEta.at(i).second.Pt(), evtWeight);
      h_MuonEta_afterPtEta->Fill(muons_afterPtEta.at(i).second.Eta(), evtWeight);
      h_MuonPhi_afterPtEta->Fill(muons_afterPtEta.at(i).second.Phi(), evtWeight);
    }

    // Pt + Eta + Id 
    options.applyIdCut = true;
    auto muons_afterPtEtaId = fMuon->GetSelectedMuons(fMuonConfig, options);

    for (int i = 0; i < muons_afterPtEtaId.size(); i++) {
      h_MuonPt_afterPtEtaId->Fill(muons_afterPtEtaId.at(i).second.Pt(), evtWeight);
      h_MuonEta_afterPtEtaId->Fill(muons_afterPtEtaId.at(i).second.Eta(), evtWeight);
      h_MuonPhi_afterPtEtaId->Fill(muons_afterPtEtaId.at(i).second.Phi(), evtWeight);
    }

    // Pt + Eta + Id + TkIso
    options.applyTkIsoCut = true;
    auto muons_afterPtEtaIdTkIso = fMuon->GetSelectedMuons(fMuonConfig, options);

    for (int i = 0; i < muons_afterPtEtaIdTkIso.size(); i++) {
      h_MuonPt_afterPtEtaIdTkIso->Fill(muons_afterPtEtaIdTkIso.at(i).second.Pt(), evtWeight);
      h_MuonEta_afterPtEtaIdTkIso->Fill(muons_afterPtEtaIdTkIso.at(i).second.Eta(), evtWeight);
      h_MuonPhi_afterPtEtaIdTkIso->Fill(muons_afterPtEtaIdTkIso.at(i).second.Phi(), evtWeight);
    }

    // auto selectedMuons = fMuon->GetSelectedMuons(fMuonConfig);

    // for (int i = 0; i < selectedMuons.size(); i++) {
    //   h_MuonPt_selected->Fill(selectedMuons.at(i).second.Pt(), evtWeight);
    //   h_MuonEta_selected->Fill(selectedMuons.at(i).second.Eta(), evtWeight);
    //   h_MuonPhi_selected->Fill(selectedMuons.at(i).second.Phi(), evtWeight);
    // }

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
      
      if (dimuon.dimuon.M() > 200.) {
        h_DimuonPt_MassCut->Fill(dimuon.dimuon.Pt(), evtWeight);
        h_DimuonRapidity_MassCut->Fill(dimuon.dimuon.Rapidity(), evtWeight);
        h_DimuonPhi_MassCut->Fill(dimuon.dimuon.Phi(), evtWeight);
        h_DimuonMass_MassCut->Fill(dimuon.dimuon.M(), evtWeight);
      }
    }
  } // End of event loop
}

void Analyzer::End()
{
  WriteHist();
}

void Analyzer::SetHist()
{
  h_MuonPt = new TH1F("h_muon_pt", "Muon pT;p_{T} [GeV];Events", 10000, 0, 10000);
  h_MuonEta = new TH1F("h_muon_eta", "Muon #eta;#eta;Events", 60, -3, 3);
  h_MuonPhi = new TH1F("h_muon_phi", "Muon #phi;#phi;Events", 24, -M_PI, M_PI);

  h_MuonPt_afterTrigger = new TH1F("h_muon_pt_after_trigger", "Muon pT after trigger;p_{T} [GeV];Events", 10000, 0, 10000);
  h_MuonEta_afterTrigger = new TH1F("h_muon_eta_after_trigger", "Muon #eta after trigger;#eta;Events", 60, -3, 3);
  h_MuonPhi_afterTrigger = new TH1F("h_muon_phi_after_trigger", "Muon #phi after trigger;#phi;Events", 24, -M_PI, M_PI);

  h_MuonPt_afterPt = new TH1F("h_muon_pt_after_pt", "Muon pT after pt cut;p_{T} [GeV];Events", 10000, 0, 10000);
  h_MuonEta_afterPt = new TH1F("h_muon_eta_after_pt", "Muon #eta after pt cut;#eta;Events", 60, -3, 3);
  h_MuonPhi_afterPt = new TH1F("h_muon_phi_after_pt", "Muon #phi after pt cut;#phi;Events", 24, -M_PI, M_PI); 

  h_MuonPt_afterPtEta = new TH1F("h_muon_pt_after_pt_eta", "Muon pT after pt and eta cuts;p_{T} [GeV];Events", 10000, 0, 10000);
  h_MuonEta_afterPtEta = new TH1F("h_muon_eta_after_pt_eta", "Muon #eta after pt and eta cuts;#eta;Events", 60, -3, 3);
  h_MuonPhi_afterPtEta = new TH1F("h_muon_phi_after_pt_eta", "Muon #phi after pt and eta cuts;#phi;Events", 24, -M_PI, M_PI);

  h_MuonPt_afterPtEtaId = new TH1F("h_muon_pt_after_pt_eta_id", "Muon pT after pt and eta and id cuts;p_{T} [GeV];Events", 10000, 0, 10000);
  h_MuonEta_afterPtEtaId = new TH1F("h_muon_eta_after_pt_eta_id", "Muon #eta after pt and eta and id cuts;#eta;Events", 60, -3, 3);
  h_MuonPhi_afterPtEtaId = new TH1F("h_muon_phi_after_pt_eta_id", "Muon #phi after pt and eta and id cuts;#phi;Events", 24, -M_PI, M_PI);

  h_MuonPt_afterPtEtaIdTkIso = new TH1F("h_muon_pt_after_pt_eta_id_tkiso", "Muon pT after pt and eta and id and tkiso cuts;p_{T} [GeV];Events", 10000, 0, 10000); 
  h_MuonEta_afterPtEtaIdTkIso = new TH1F("h_muon_eta_after_pt_eta_id_tkiso", "Muon #eta after pt and eta and id and tkiso cuts;#eta;Events", 60, -3, 3);
  h_MuonPhi_afterPtEtaIdTkIso = new TH1F("h_muon_phi_after_pt_eta_id_tkiso", "Muon #phi after pt and eta and id and tkiso cuts;#phi;Events", 24, -M_PI, M_PI);

  // h_MuonPt_selected = new TH1F("h_muon_selected_pt", "Muon pT;p_{T} [GeV];Events", 10000, 0, 10000);
  // h_MuonEta_selected = new TH1F("h_muon_selected_eta", "Muon #eta;#eta;Events", 60, -3, 3);
  // h_MuonPhi_selected = new TH1F("h_muon_selected_phi", "Muon #phi;#phi;Events", 24, -M_PI, M_PI);

  h_SingleMuonPt = new TH1F("h_singlemuon_pt", "Single muon pT;p_{T} [GeV];Events", 10000, 0, 10000);
  h_SingleMuonEta = new TH1F("h_singlemuon_eta", "Single muon #eta;#eta;Events", 60, -3, 3);
  h_SingleMuonPhi = new TH1F("h_singlemuon_phi", "Single muon #phi;#phi;Events", 24, -M_PI, M_PI);

  h_LeadingMuonPt = new TH1F("h_leadingmuon_pt", "Leading muon pT;p_{T} [GeV];Events", 10000, 0, 10000);
  h_LeadingMuonEta = new TH1F("h_leadingmuon_eta", "Leading muon #eta;#eta;Events", 60, -3, 3);
  h_LeadingMuonPhi = new TH1F("h_leadingmuon_phi", "Leading muon #phi;#phi;Events", 24, -M_PI, M_PI);

  h_SubleadingMuonPt = new TH1F("h_subleadingmuon_pt", "Subleading muon pT;p_{T} [GeV];Events", 10000, 0, 10000);
  h_SubleadingMuonEta = new TH1F("h_subleadingmuon_eta", "Subleading muon #eta;#eta;Events", 60, -3, 3);
  h_SubleadingMuonPhi = new TH1F("h_subleadingmuon_phi", "Subleading muon #phi;#phi;Events", 24, -M_PI, M_PI);

  h_DimuonPt = new TH1F("h_dimuon_pt", "Dimuon pT;p_{T} [GeV];Events", 10000, 0, 10000);
  h_DimuonRapidity = new TH1F("h_dimuon_rapidity", "Dimuon rapidity;y;Events", 60, -3, 3);
  h_DimuonPhi = new TH1F("h_dimuon_phi", "Dimuon #phi;#phi;Events", 24, -M_PI, M_PI);
  h_DimuonMass = new TH1F("h_dimuon_mass", "Dimuon mass;m [GeV];Events", 10000, 0, 10000);

  h_DimuonPt_MassCut = new TH1F("h_dimuon_pt_mass_cut", "Dimuon pT;p_{T} [GeV];Events", 10000, 0, 10000);
  h_DimuonRapidity_MassCut = new TH1F("h_dimuon_rapidity_mass_cut", "Dimuon rapidity;y;Events", 60, -3, 3);
  h_DimuonPhi_MassCut = new TH1F("h_dimuon_phi_mass_cut", "Dimuon #phi;#phi;Events", 24, -M_PI, M_PI);
  h_DimuonMass_MassCut = new TH1F("h_dimuon_mass_mass_cut", "Dimuon mass;m [GeV];Events", 10000, 0, 10000);
}

void Analyzer::FillHist()
{
  // will add later.
}

void Analyzer::WriteHist()
{
  TFile outputFile(fOutputName.c_str(), "RECREATE");
  
  h_MuonPt->Write();
  h_MuonEta->Write();
  h_MuonPhi->Write();

  h_MuonPt_afterTrigger->Write();
  h_MuonEta_afterTrigger->Write();
  h_MuonPhi_afterTrigger->Write();

  h_MuonPt_afterPt->Write();
  h_MuonEta_afterPt->Write();
  h_MuonPhi_afterPt->Write();

  h_MuonPt_afterPtEta->Write();
  h_MuonEta_afterPtEta->Write();
  h_MuonPhi_afterPtEta->Write();

  h_MuonPt_afterPtEtaId->Write();
  h_MuonEta_afterPtEtaId->Write();
  h_MuonPhi_afterPtEtaId->Write();

  h_MuonPt_afterPtEtaIdTkIso->Write();
  h_MuonEta_afterPtEtaIdTkIso->Write();
  h_MuonPhi_afterPtEtaIdTkIso->Write();

  // h_MuonPt_selected->Write();
  // h_MuonEta_selected->Write();
  // h_MuonPhi_selected->Write();

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

  h_DimuonPt_MassCut->Write();
  h_DimuonRapidity_MassCut->Write();
  h_DimuonPhi_MassCut->Write();
  h_DimuonMass_MassCut->Write();

  outputFile.Close();
  
  std::cout << "Output saved to: " << fOutputName << std::endl;
}
