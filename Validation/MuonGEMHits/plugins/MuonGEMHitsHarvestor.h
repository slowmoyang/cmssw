#ifndef VALIDATION_MUONGEMHITS_PLUGINS_MUONGEMHITSHARVESTOR_H_
#define VALIDATION_MUONGEMHITS_PLUGINS_MUONGEMHITSHARVESTOR_H_

#include "Validation/MuonGEMHits/interface/MuonGEMBaseHarvestor.h"

class MuonGEMHitsHarvestor : public MuonGEMBaseHarvestor {
 public:
  explicit MuonGEMHitsHarvestor(const edm::ParameterSet &);
  ~MuonGEMHitsHarvestor() override;
  void dqmEndJob(DQMStore::IBooker &, DQMStore::IGetter &) override;
  
 private:
  std::string folder_;
};

#endif // VALIDATION_MUONGEMHITS_PLUGINS_MUONGEMHITSHARVESTOR_H_
