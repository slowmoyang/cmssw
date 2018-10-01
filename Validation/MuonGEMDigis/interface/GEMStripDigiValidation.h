#ifndef VALIDATION_MUONGEMDIGIS_GEMSTRIPDIGIVALIDATION_H_
#define VALIDATION_MUONGEMDIGIS_GEMSTRIPDIGIVALIDATION_H_

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"

class GEMStripDigiValidation : public GEMBaseValidation {
 public:
  explicit GEMStripDigiValidation(const edm::ParameterSet &);
  ~GEMStripDigiValidation() override;
  void analyze(const edm::Event &,
               const edm::EventSetup &) override;
  void bookHistograms(DQMStore::IBooker &,
                      edm::Run const &,
                      edm::EventSetup const &) override;

 private:
  // Simple plots
  MEMap1Ids me_occ_zr_; // key = region_id
  MEMap2Ids me_occ_det_;

  // Detail plots
  MEMap3Ids me_detail_occ_zr_;
  MEMap3Ids me_detail_occ_xy_;
  MEMap3Ids me_detail_occ_phi_strip_;
  MEMap3Ids me_detail_occ_strip_;
  MEMap3Ids me_detail_bx_;

  // NOTE muon simhit - strip digi matching
  // occupancy plots for efficiency
  MEMap1Ids me_simhit_occ_eta_;
  MEMap1Ids me_strip_occ_eta_;
  MEMap2Ids me_simhit_occ_phi_;
  MEMap2Ids me_strip_occ_phi_;
  MEMap2Ids me_simhit_occ_det_;
  MEMap2Ids me_strip_occ_det_;

  // ParameterSet
  edm::EDGetToken simhit_token_;
  edm::EDGetToken strip_token_;
};

#endif // VALIDATION_MUONGEMDIGIS_GEMSTRIPDIGIVALIDATION_H_
