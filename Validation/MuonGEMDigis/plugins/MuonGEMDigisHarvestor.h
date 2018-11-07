#ifndef VALIDATION_MUONGEMDIGIS_PLUGINS_MUONGEMDIGISHARVESTOR_H_
#define VALIDATION_MUONGEMDIGIS_PLUGINS_MUONGEMDIGISHARVESTOR_H_

#include "Validation/MuonGEMHits/interface/MuonGEMBaseHarvestor.h"

class MuonGEMDigisHarvestor : public MuonGEMBaseHarvestor {
 public:
  explicit MuonGEMDigisHarvestor(const edm::ParameterSet&);
  ~MuonGEMDigisHarvestor() override;
  void dqmEndJob(DQMStore::IBooker &, DQMStore::IGetter &) override;

  // TODO
  // void harvestStripDigi();
  // void harvestPadDigi();
  // void harvestCoPadDigi();

 private:
  std::string strip_folder_, pad_folder_, copad_folder_;
  std::vector<Int_t> region_ids_, station_ids_, layer_ids_;
};

#endif // VALIDATION_MUONGEMDIGIS_PLUGINS_MUONGEMDIGISHARVESTOR_H_
