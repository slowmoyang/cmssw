#ifndef Validation_MuonGEMHits_GEMHitsValidation_h_
#define Validation_MuonGEMHits_GEMHitsValidation_h_

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"

#include <tuple>
#include <map>
#include <vector>
#include <string>

class GEMHitsValidation : public GEMBaseValidation {
 public:
  explicit GEMHitsValidation(const edm::ParameterSet &);
  ~GEMHitsValidation() override;
  void bookHistograms(DQMStore::IBooker &,
                      edm::Run const &,
                      edm::EventSetup const &) override;
  void analyze(const edm::Event & event, const edm::EventSetup &) override;

 private:
  std::tuple<Double_t, Double_t> getTOFRange(Int_t station_id);

  // Parameters
  edm::EDGetToken simhit_token_;
  std::vector<Double_t> tof_range_; 

  // Monitor elemnts
  // Time of Flight
  std::map<Int_t, MonitorElement*> me_tof_mu_;
  MEMap3Ids me_detail_tof_;
  MEMap3Ids me_detail_tof_mu_;

  // Energy loss
  MEMap1Ids me_eloss_mu_;
  MEMap3Ids me_detail_eloss_;
  MEMap3Ids me_detail_eloss_mu_;

  // Occupancy
  MEMap1Ids me_occ_zr_;
  MEMap2Ids me_occ_det_;
  MEMap3Ids me_occ_xy_;

  // Constants
  // energy loss conversion factor:
  const Float_t kEnergyCF_ = 1e9f;

};

#endif // Validation_MuonGEMHits_GEMHitsValidation_h_
