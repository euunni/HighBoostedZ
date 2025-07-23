#ifndef EFFSF_H
#define EFFSF_H

#include <string>
#include <memory>
#include <map>
#include <vector>
#include "TH2.h"
#include <TLorentzVector.h>


class EffSF {
public:
  EffSF(const std::string& idFile,
        const std::string& isoFile,
        const std::string& triggerFile,
        const std::map<std::string, std::string>& histNames = {});

  double GetIDSF(const TLorentzVector& muon) const;
  double GetISOSF(const TLorentzVector& muon) const;
  double GetTrigSF(const TLorentzVector& muon1, const TLorentzVector& muon2) const;

private:
  std::unique_ptr<TH2> fID;
  std::unique_ptr<TH2> fISO;
  std::unique_ptr<TH2> fTriggerData;
  std::unique_ptr<TH2> fTriggerMC;

  static std::unique_ptr<TH2> LoadHist(const std::string& filename, const std::string& histName);
  static double GetEff(const TH2* hist, double pt, double eta);
};

#endif // EFFSF_H 