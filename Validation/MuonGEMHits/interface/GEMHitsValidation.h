#ifndef VALIDATION_MUONGEMHITS_INTERFACE_GEMHITSVALIDATION_H_
#define VALIDATION_MUONGEMHITS_INTERFACE_GEMHITSVALIDATION_H_

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

  // Simple plots
  MEMap1Ids me_occ_zr_; // [regions]
  std::map<Int_t, MonitorElement*> me_tof_mu_; // [stations]
  MEMap1Ids me_eloss_mu_; // [stations]
  MEMap2Ids me_occ_det_; // [r][s]

  // Detail plots
  // [regions][stations][layers]
  MEMap3Ids me_detail_occ_xy_;
  MEMap3Ids me_detail_occ_zr_;
  MEMap3Ids me_detail_tof_;
  MEMap3Ids me_detail_tof_mu_;
  MEMap3Ids me_detail_eloss_;
  MEMap3Ids me_detail_eloss_mu_;

  MonitorElement* me_gem_geom_xyz_;
  MonitorElement* me_gem_geom_eta_phi_;

  // energy loss conversion factor:
  const Float_t kEnergyCF_ = 1.e9;

  edm::EDGetToken simhit_token_;
  std::vector<Double_t> tof_range_; 
};

#endif // VALIDATION_MUONGEMHITS_INTERFACE_GEMHITSVALIDATION_H_
