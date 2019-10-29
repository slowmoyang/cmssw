#ifndef Validation_MuonGEMHits_MuonGEMBaseHarvestor_h_
#define Validation_MuonGEMHits_MuonGEMBaseHarvestor_h_

#include "DQMServices/Core/interface/DQMEDHarvester.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"

#include "TSystem.h"
#include "TString.h"

class MuonGEMBaseHarvestor : public DQMEDHarvester {
 public:
  explicit MuonGEMBaseHarvestor(const edm::ParameterSet &);

 protected:
  template <typename T>
  T* getElement(DQMStore::IGetter & getter,
                const TString & path);

  // 0.6893 means 1 standard deviation of the normal distribution.
  TProfile* computeEfficiency(const TH1F & passed,
                              const TH1F & total,
                              const char* name,
                              const char* title,
                              Double_t confidence_level=0.683);

  TH2F* computeEfficiency(const TH2F & passed,
                          const TH2F & total,
                          const char* name,
                          const char* title);

  void bookEff1D(DQMStore::IBooker & booker,
                 DQMStore::IGetter & getter,
                 const TString & passed_path,
                 const TString & total_path,
                 const TString & folder,
                 const TString & eff_name,
                 const TString & eff_title="Efficiency");

  void bookEff2D(DQMStore::IBooker & booker,
                 DQMStore::IGetter & getter,
                 const TString & passed_path,
                 const TString & total_path,
                 const TString & folder,
                 const TString & eff_name,
                 const TString & eff_title="Efficiency");

  std::string log_category_;
};


#include "Validation/MuonGEMHits/src/MuonGEMBaseHarvestor.tpp"

#endif // Validation_MuonGEMHits_MuonGEMBaseHarvestor_h_
