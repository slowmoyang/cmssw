#ifndef Validation_MuonGEMDigis_GEMCoPadDigiValidation_H_
#define Validation_MuonGEMDigis_GEMCoPadDigiValidation_H_

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"


class GEMCoPadDigiValidation : public GEMBaseValidation {
 public:
  explicit GEMCoPadDigiValidation(const edm::ParameterSet & ps);
  ~GEMCoPadDigiValidation() override;
  void analyze(const edm::Event & event,
               const edm::EventSetup & setup) override;
  void bookHistograms(DQMStore::IBooker & ibooker,
                      edm::Run const & run,
                      edm::EventSetup const & setup) override;

 private:
  // ParameterSet
  edm::EDGetToken copad_token_;
  Int_t gem_bx_min_, gem_bx_max_;

  MEMap1Ids me_occ_zr_;
  MEMap2Ids me_occ_det_;
  MEMap2Ids me_detail_occ_xy_;
  MEMap2Ids me_detail_occ_phi_pad_;
  MEMap2Ids me_detail_occ_pad_;

  MEMap2Ids me_detail_bx_;

};

#endif // Validation_MuonGEMDigis_GEMCoPadDigiValidation_H_
