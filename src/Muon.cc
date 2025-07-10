#include "Muon.h"

#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TLorentzVector.h"
#include "TTree.h"
#include "TRandom3.h"


void Muon::Init(TTreeReader* fReader) {
  Muon_pt = new TTreeReaderArray<float>(*fReader, "Muon_pt");
  Muon_eta = new TTreeReaderArray<float>(*fReader, "Muon_eta");
  Muon_phi = new TTreeReaderArray<float>(*fReader, "Muon_phi");
  Muon_mass = new TTreeReaderArray<float>(*fReader, "Muon_mass");
  Muon_charge = new TTreeReaderArray<int>(*fReader, "Muon_charge");
  Muon_tightId = new TTreeReaderArray<bool>(*fReader, "Muon_tightId");
  Muon_pfRelIso04_all = new TTreeReaderArray<float>(*fReader, "Muon_pfRelIso04_all");
  Muon_nTrackerLayers = new TTreeReaderArray<int>(*fReader, "Muon_nTrackerLayers");

  // Optional branches for gen-particle matching
  if (fReader->GetTree() && fReader->GetTree()->GetBranch("Muon_genPartIdx")) {
    Muon_genPartIdx = new TTreeReaderArray<int>(*fReader, "Muon_genPartIdx");
  }
  if (fReader->GetTree() && fReader->GetTree()->GetBranch("GenPart_pt")) {
    GenPart_pt = new TTreeReaderArray<float>(*fReader, "GenPart_pt");
  }

  const std::vector<std::string> commonTriggers = {"HLT_IsoMu24", "HLT_IsoTkMu24", "HLT_IsoMu27"};
  
  for (const auto& trigger : commonTriggers) {
    if (fReader->GetTree() && fReader->GetTree()->GetBranch(trigger.c_str())) {
      triggerMap[trigger] = new TTreeReaderValue<bool>(*fReader, trigger.c_str());
    }
  }
}

void Muon::SetRoccoR(const RoccoR* rc, bool isMC, bool applyRoch) {
  fRoccor = rc;
  fIsMC = isMC;
  fApplyRoch = applyRoch;
}

std::vector<TLorentzVector> Muon::Get4Vec(bool doRoch) {
  fMuon4Vec.clear();
  fMuon4VecRaw.clear();

  for (int i = 0; i < Muon_pt->GetSize(); i++) {
    float pt   = Muon_pt->At(i);
    float eta  = Muon_eta->At(i);
    float phi  = Muon_phi->At(i);
    float mass = Muon_mass->At(i);

    TLorentzVector muRaw;
    muRaw.SetPtEtaPhiM(pt, eta, phi, mass);
    fMuon4VecRaw.push_back(muRaw);

    // Rochester scale factor (sf)
    double sf = 1.0;

    if (fRoccor) {
      int Q = Muon_charge->At(i);
      
      if (fIsMC) {
        bool hasGen = (Muon_genPartIdx != nullptr && GenPart_pt != nullptr);
        int genIdx = hasGen ? Muon_genPartIdx->At(i) : -1;

        if (genIdx >= 0) { // matched gen muon
          double genPt = GenPart_pt->At(genIdx);
          sf = fRoccor->kSpreadMC(Q, pt, eta, phi, genPt, 5, 0);
        } else { // unmatched gen muon
          int nl = Muon_nTrackerLayers->At(i);
          double u = gRandom->Rndm();
          sf = fRoccor->kSmearMC(Q, pt, eta, phi, nl, u, 5, 0);
        }
      } else { // data
        sf = fRoccor->kScaleDT(Q, pt, eta, phi, 5, 0);
      }
    }

    TLorentzVector muCorr = muRaw;
    muCorr *= sf;
    fMuon4Vec.push_back(muCorr);
  }

  return doRoch ? fMuon4Vec : fMuon4VecRaw;
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

std::vector<std::pair<int, TLorentzVector>> Muon::GetSelectedMuons(const Selection& config) {
  std::vector<std::pair<int, TLorentzVector>> selectedMuons;
  selectedMuons.reserve(fMuon4Vec.size());
  Get4Vec(fApplyRoch);
  
  for (size_t i = 0; i < fMuon4Vec.size(); i++) {
    const auto& muon = fMuon4Vec[i];

    if (muon.Pt() > config.Subleading_Pt && 
        std::abs(muon.Eta()) < config.Eta &&
        Muon_tightId->At(i) &&
        Muon_pfRelIso04_all->At(i) < config.PFIso) {
      selectedMuons.push_back({i, muon});
    }
  }

  return selectedMuons;
}

DimuonPair Muon::GetDimuon(const Selection& config) {
  DimuonPair dimuon;
  auto selectedMuons = GetSelectedMuons(config);
  
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
    dimuon.leadIdx = leadIdx;
    dimuon.subIdx  = subleadIdx;
    dimuon.leading = &fMuon4Vec[leadIdx];
    dimuon.subleading = &fMuon4Vec[subleadIdx];
    dimuon.dimuon = *dimuon.leading + *dimuon.subleading;

    double dimuonMass = dimuon.dimuon.M();
    if (dimuonMass > config.ZMass[0] && dimuonMass < config.ZMass[1]) {
      dimuon.isValid = true;
    }
  }
  
  return dimuon;
}
