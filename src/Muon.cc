#include "Muon.h"

#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TLorentzVector.h"
#include "TTree.h"


void Muon::Init(TTreeReader* fReader) {

  Muon_pt = new TTreeReaderArray<float>(*fReader, "Muon_pt");
  Muon_eta = new TTreeReaderArray<float>(*fReader, "Muon_eta");
  Muon_phi = new TTreeReaderArray<float>(*fReader, "Muon_phi");
  Muon_mass = new TTreeReaderArray<float>(*fReader, "Muon_mass");
  Muon_charge = new TTreeReaderArray<int>(*fReader, "Muon_charge");
  Muon_tightId = new TTreeReaderArray<bool>(*fReader, "Muon_tightId");
  Muon_pfRelIso04_all = new TTreeReaderArray<float>(*fReader, "Muon_pfRelIso04_all");
  Muon_nTrackerLayers = new TTreeReaderArray<int>(*fReader, "Muon_nTrackerLayers");

  const std::vector<std::string> commonTriggers = {"HLT_IsoMu24", "HLT_IsoTkMu24", "HLT_IsoMu27"};
  TTree* tree = fReader->GetTree();
  
  for (const auto& trigger : commonTriggers) {
    if (tree && tree->GetBranch(trigger.c_str())) {
      triggerMap[trigger] = new TTreeReaderValue<bool>(*fReader, trigger.c_str());
    }
  }
}

std::vector<TLorentzVector> Muon::Get4Vec() {
  fMuon4Vec.clear();
  
  for (int i = 0; i < Muon_pt->GetSize(); i++) {
    float pt = Muon_pt->At(i);
    float eta = Muon_eta->At(i);
    float phi = Muon_phi->At(i);
    float mass = Muon_mass->At(i);

		TLorentzVector muon;
		muon.SetPtEtaPhiM(pt, eta, phi, mass);
		fMuon4Vec.push_back(muon);
	}
	return fMuon4Vec;
}

std::vector<int> Muon::GetCharge() {
  fMuonCharge.clear();
  
  for (int i = 0; i < Muon_charge->GetSize(); i++) {
    fMuonCharge.push_back(Muon_charge->At(i));
  }
  return fMuonCharge;
} 

std::vector<std::string> Muon::GetTriggers(const Selection& config, const std::string& sampleName) {
  std::vector<std::string> defaultTriggers = config.j["Muon"]["Trigger"]["Default"].get<std::vector<std::string>>();
  
  if (config.j["Muon"]["Trigger"].contains("Exception")) {
    auto& exceptions = config.j["Muon"]["Trigger"]["Exception"];
    if (exceptions.contains(sampleName)) {
      return exceptions[sampleName].get<std::vector<std::string>>();
    }
  }

  return defaultTriggers;
}

bool Muon::PassTriggers(const std::vector<std::string>& triggerList) {
  for (const auto& trigger : triggerList) {
    auto it = triggerMap.find(trigger);
    if (it != triggerMap.end() && *(*it->second)) {
      return true;
    }
  }
  return false;
}

std::vector<std::pair<int, TLorentzVector>> Muon::GetSelectedMuons(const Selection& config, const SelectionOptions& options = SelectionOptions()) {
  std::vector<std::pair<int, TLorentzVector>> selectedMuons;
  selectedMuons.reserve(fMuon4Vec.size());
  Get4Vec();
  
  for (size_t i = 0; i < fMuon4Vec.size(); i++) {
    const auto& muon = fMuon4Vec[i];
    bool passSelection = true;

    if (options.applyPtCut && muon.Pt() <= config.Subleading_Pt) passSelection = false;
    if (options.applyEtaCut && std::abs(muon.Eta()) >= config.Eta) passSelection = false;
    if (options.applyIdCut && !Muon_tightId->At(i)) passSelection = false;
    if (options.applyPFIsoCut && Muon_pfRelIso04_all->At(i) >= config.PFIso) passSelection = false;
    if (passSelection) {
      selectedMuons.push_back({i, muon});
    }
  }

  return selectedMuons;
}

DimuonPair Muon::GetDimuon(const Selection& config, const SelectionOptions& options = SelectionOptions()) {
  DimuonPair dimuon;
  auto selectedMuons = GetSelectedMuons(config, options);
  
  GetCharge(); 

  int leadIdx = -1, subleadIdx = -1;

  if (selectedMuons.size() >= 2) {
    std::sort(selectedMuons.begin(), selectedMuons.end(),
              [](const auto& a, const auto& b) { return a.second.Pt() > b.second.Pt(); });
    
    if (selectedMuons[0].second.Pt() > config.Leading_Pt) {
      leadIdx = selectedMuons[0].first;

      for (int i = 1; i < selectedMuons.size(); i++) {
        int idx = selectedMuons[i].first;
        if (fMuonCharge[idx] * fMuonCharge[leadIdx] < 0) {
          subleadIdx = idx;
          break;
        }
      }
    }
  }

  if (leadIdx != -1 && subleadIdx != -1) {
    dimuon.leading = &fMuon4Vec[leadIdx];
    dimuon.subleading = &fMuon4Vec[subleadIdx];
    dimuon.dimuon = *dimuon.leading + *dimuon.subleading;

    if (options.applyZMassCut) {
      double dimuonMass = dimuon.dimuon.M();
      if (!(dimuonMass > config.ZMass[0] && dimuonMass < config.ZMass[1])) {
        dimuon.isValid = false;
        return dimuon;
      }
    }
    
    dimuon.isValid = true;
  }
  
  return dimuon;
}
