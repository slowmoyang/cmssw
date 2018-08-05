#ifndef VALIDATION_MUONGEMDIGIS_PLUGINS_MUONGEMDIGISHARVESTOR_H_
#define VALIDATION_MUONGEMDIGIS_PLUGINS_MUONGEMDIGISHARVESTOR_H_

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/DQMEDHarvester.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"

class TProfile;

class MuonGEMDigisHarvestor : public DQMEDHarvester {
 public:
  explicit MuonGEMDigisHarvestor(const edm::ParameterSet&);
  ~MuonGEMDigisHarvestor() override;
  void dqmEndJob(DQMStore::IBooker &, DQMStore::IGetter &) override;

  // TODO
  // void harvestStripDigi();
  // void harvestPadDigi();
  // void harvestCoPadDigi();

  TProfile* computeEff(const TH1F & passed, const TH1F & total);

 private:
  std::string dbe_path_;
  std::string strip_digi_path_;
  std::vector<int> region_ids_, station_ids_, layer_ids_;

  // Coonst
  const std::string kLogCategory_ = "MuonGEMDigisHarvestor";
  const std::vector<const char*> kAxes_ = {"eta", "phi"};
};

#endif // VALIDATION_MUONGEMDIGIS_PLUGINS_MUONGEMDIGISHARVESTOR_H_
