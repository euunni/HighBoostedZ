#ifndef Muon_h
#define Muon_h

#include <iostream>
#include <algorithm>
#include <fstream>
#include "TTreeReaderArray.h"
#include "TTreeReader.h"
#include "TLorentzVector.h"
#include <nlohmann/json.hpp>
#include <map>

using json = nlohmann::json;


struct SelectionOptions {
  bool applyPtCut = false;
  bool applyEtaCut = false;
  bool applyIdCut = false;
  bool applyPFIsoCut = false;
  bool applyZMassCut = false;
};

struct Selection {
  double Leading_Pt;
  double Subleading_Pt;
  double Eta;
  std::string Id;
  double PFIso;
  std::vector<double> ZMass;
  json j;

  static Selection Load(const std::string& filename) {
    Selection config;
    std::ifstream file(filename);
    file >> config.j;
    
    config.Leading_Pt = config.j["Muon"]["Leading_Pt"].get<double>();
    config.Subleading_Pt = config.j["Muon"]["Subleading_Pt"].get<double>();
    config.Eta = config.j["Muon"]["Eta"].get<double>();
    config.Id = config.j["Muon"]["Id"].get<std::string>();  
    config.PFIso = config.j["Muon"]["PFIso"].get<double>();
    config.ZMass = config.j["Muon"]["ZMass"].get<std::vector<double>>();
    return config;
  }
};

struct DimuonPair {
  const TLorentzVector* leading = nullptr;
  const TLorentzVector* subleading = nullptr;
  TLorentzVector dimuon;
  bool isValid = false;
};

class Muon
{
public:
  Muon() {
    Muon_pt = nullptr;
    Muon_eta = nullptr;
    Muon_phi = nullptr;
    Muon_mass = nullptr;
    Muon_charge = nullptr;
  }
  ~Muon() {
    for (auto& pair : triggerMap) {
      delete pair.second;
      pair.second = nullptr;
    }
  }

  void Init(TTreeReader* fReader);
  std::vector<TLorentzVector> Get4Vec();
  std::vector<int> GetCharge();
  std::vector<std::string> GetTriggers(const Selection& config, const std::string& sampleName);
  bool PassTriggers(const std::vector<std::string>& triggerList);
  std::vector<std::pair<int, TLorentzVector>> GetSelectedMuons(const Selection& config);
  DimuonPair GetDimuon(const Selection& config);

private:
  TTreeReaderArray<float>* Muon_pt;
  TTreeReaderArray<float>* Muon_eta;
  TTreeReaderArray<float>* Muon_phi;
  TTreeReaderArray<float>* Muon_mass;
  TTreeReaderArray<int>* Muon_charge;
  TTreeReaderArray<bool>* Muon_tightId;
  TTreeReaderArray<float>* Muon_pfRelIso04_all;
  TTreeReaderArray<int>* Muon_nTrackerLayers;

  std::map<std::string, TTreeReaderValue<bool>*> triggerMap;
  std::vector<TLorentzVector> fMuon4Vec;
  std::vector<int> fMuonCharge;
};

#endif
