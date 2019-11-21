#ifndef Validation_MuonGEMDigis_GEMStripDigiValidation_h_
#define Validation_MuonGEMDigis_GEMStripDigiValidation_h_

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
  // ParameterSet
  edm::EDGetToken simhit_token_;
  edm::EDGetToken strip_token_;

  // Monitor elements
  MEMap1Ids me_occ_zr_;
  MEMap2Ids me_occ_det_;
  MEMap3Ids me_detail_occ_xy_;
  MEMap3Ids me_detail_occ_phi_strip_;
  MEMap3Ids me_detail_occ_strip_;

  // NOTE muon simhit - strip digi matching
  // occupancy plots for efficiency
  MEMap1Ids me_simhit_occ_eta_;
  MEMap1Ids me_strip_occ_eta_;
  MEMap2Ids me_simhit_occ_phi_;
  MEMap2Ids me_strip_occ_phi_;
  MEMap2Ids me_simhit_occ_det_;
  MEMap2Ids me_strip_occ_det_;

  // bunch crossing
  MEMap3Ids me_detail_bx_;
};

#endif // Validation_MuonGEMDigis_GEMStripDigiValidation_h_
