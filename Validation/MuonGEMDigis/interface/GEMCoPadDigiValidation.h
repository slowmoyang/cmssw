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
  // Simple plots
  MEMap1Ids me_occ_zr_; // key = region_id
  MEMap2Ids me_occ_det_; // key(region_id, station_id)

  // DetailPlots
  // key(region_id, station_id)
  MEMap2Ids me_detail_occ_xy_;
  MEMap2Ids me_detail_occ_zr_;
  // key(region_id, station_id)
  MEMap2Ids me_detail_occ_phi_pad_;
  MEMap2Ids me_detail_occ_pad_;
  MEMap2Ids me_detail_bx_;

  // ParameterSet
  edm::EDGetToken copad_token_;
  // FIXME
  Int_t minBXGEM_, maxBXGEM_;
  Bool_t detail_plot_;
  // Custom Constants
};

#endif // VALIDATION_MUONGEMDIGIS_GEMCOPADDIGIVALIDATION_H_
