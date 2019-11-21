#ifndef Validation_MuonGEMDigis_GEMPadDigiValidation_H_
#define Validation_MuonGEMDigis_GEMPadDigiValidation_H_

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"


class GEMPadDigiValidation : public GEMBaseValidation {
 public:
  explicit GEMPadDigiValidation( const edm::ParameterSet& );
  ~GEMPadDigiValidation() override;
  void analyze(const edm::Event& e,
               const edm::EventSetup&) override;
  void bookHistograms(DQMStore::IBooker &,
                      edm::Run const &,
                      edm::EventSetup const &) override;

 private:
  // parameters
  edm::EDGetToken pad_token_;

  // monitor elements
  MEMap2Ids me_occ_det_;
  MEMap1Ids me_occ_zr_;
  MEMap3Ids me_detail_occ_xy_; 
  MEMap3Ids me_detail_occ_phi_pad_;
  MEMap3Ids me_detail_occ_pad_; // DIGI Occupancy per Pad number

  MEMap3Ids me_detail_bx_;
};

#endif // Validation_MuonGEMDigis_GEMPadDigiValidation_H_
