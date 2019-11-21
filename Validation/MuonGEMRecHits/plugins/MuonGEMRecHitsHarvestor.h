#ifndef VALIDATION_MUONGEMRECHITS_PLUGINS_MUONGEMRECHITSHARVESTOR_H_
#define VALIDATION_MUONGEMRECHITS_PLUGINS_MUONGEMRECHITSHARVESTOR_H_

#include "Validation/MuonGEMHits/interface/MuonGEMBaseHarvestor.h"

class MuonGEMRecHitsHarvestor : public MuonGEMBaseHarvestor {
 public:
  explicit MuonGEMRecHitsHarvestor(const edm::ParameterSet&);
  ~MuonGEMRecHitsHarvestor() override;
  void dqmEndJob(DQMStore::IBooker &, DQMStore::IGetter &) override;

  std::vector<Int_t> region_ids_, station_ids_, layer_ids_;
  std::string folder_;
};

#endif // VALIDATION_MUONGEMRECHITS_PLUGINS_MUONGEMRECHITSHARVESTOR_H_
