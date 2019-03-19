#ifndef VALIDATION_MUONGEMDIGIS_GEMCOPADDIGIVALIDATION_H_
#define VALIDATION_MUONGEMDIGIS_GEMCOPADDIGIVALIDATION_H_

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

#endif // VALIDATION_MUONGEMDIGIS_GEMCOPADDIGIVALIDATION_H_
