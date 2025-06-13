#ifndef PUREWEIGHTING_H
#define PUREWEIGHTING_H

#include <TH1D.h>
#include <vector>
#include <string>
#include <memory>

class PUReweighting {
public:
  PUReweighting(const std::string& dataPUFile, const std::string& mcPUFile);
  double GetWeight(int puValue) const;
  TH1D* GetDataHist() const { return fHData.get(); }
  TH1D* GetMCHist() const { return fHMC.get(); }
  TH1D* GetMCWeightedHist();

private:
  std::vector<double> fPU;
  std::unique_ptr<TH1D> fHData;
  std::unique_ptr<TH1D> fHMC;
  std::unique_ptr<TH1D> fHMCReweighted;
  void CalcWeight(const std::string& dataPUFile, const std::string& mcPUFile);
};

#endif // PUREWEIGHTING_H
