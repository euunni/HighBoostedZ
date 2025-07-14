#ifndef EFFSF_H
#define EFFSF_H

#include <string>
#include <memory>
#include <map>
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
  struct HistPair {
    std::unique_ptr<TH2> hData;
    std::unique_ptr<TH2> hMC;
  };

  HistPair fID;
  HistPair fISO;
  HistPair fTrigger;

  static HistPair LoadHist(const std::string& filename, const std::string& dataHistName, const std::string& mcHistName);
  static double GetEff(const TH2* hist, double pt, double eta);
  static double GetSF(const TH2* hData, const TH2* hMC, double pt, double eta);
};

#endif // EFFSF_H 