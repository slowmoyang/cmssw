#ifndef VALIDATION_MUONGEMHITS_MUONGEMBASESHARVESTOR_H_
#define VALIDATION_MUONGEMHITS_MUONGEMBASESHARVESTOR_H_

#include "DQMServices/Core/interface/DQMEDHarvester.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"

#include "TSystem.h"
#include "TString.h"

#include <string>

class MuonGEMBaseHarvestor : public DQMEDHarvester {
 public:
  explicit MuonGEMBaseHarvestor(const edm::ParameterSet &);

 protected:
  template <typename T>
  T* getElement(DQMStore::IGetter & igetter,
                const std::string & folder,
                const TString & name);

  TProfile* computeEfficiency(const TH1F & passed,
                              const TH1F & total,
                              const char* name,
                              const char* title,
                              Double_t confidence_level=0.683);

  TH2F* computeEfficiency(const TH2F & passed,
                          const TH2F & total,
                          const char* name,
                          const char* title);

  void bookEff1D(DQMStore::IBooker & ibooker,
                 DQMStore::IGetter & igetter,
                 const std::string & folder,
                 const TString & passed_name,
                 const TString & total_name,
                 const TString & eff_name,
                 const TString & eff_title="Efficiency");

  void bookEff2D(DQMStore::IBooker & ibooker,
                 DQMStore::IGetter & igetter,
                 const std::string & folder,
                 const TString & passed_name,
                 const TString & total_name,
                 const TString & eff_name,
                 const TString & eff_title="Efficiency");

  std::string log_category_;
};


#include "Validation/MuonGEMHits/src/MuonGEMBaseHarvestor.tpp"

#endif // VALIDATION_MUONGEMHITS_PLUGINS_MUONGEMBASESHARVESTOR_H_
