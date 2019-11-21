#ifndef Validation_MuonGEMRecHits_GEMRecHitsValidation_H_
#define Validation_MuonGEMRecHits_GEMRecHitsValidation_H_

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

class GEMRecHitsValidation : public GEMBaseValidation {
 public:
  explicit GEMRecHitsValidation(const edm::ParameterSet &);
  ~GEMRecHitsValidation() override;
  void analyze(const edm::Event &,
               const edm::EventSetup &) override;
  void bookHistograms(DQMStore::IBooker &,
                      edm::Run const &,
                      edm::EventSetup const &) override;

 private:
  Bool_t matchRecHitAgainstSimHit(GEMRecHitCollection::const_iterator, Int_t);

  // ParameterSet
  edm::EDGetToken simhit_token_;
  edm::EDGetToken rechit_token_;

  // MonitorElement

  // cluster size of rechit
  MonitorElement* me_cls_;
  MEMap3Ids me_detail_cls_;

  MEMap1Ids me_pull_x_;
  MEMap1Ids me_pull_y_;
  MEMap3Ids me_detail_pull_x_;
  MEMap3Ids me_detail_pull_y_;

  MEMap1Ids me_residual_x_;
  MEMap1Ids me_residual_y_;
  MEMap3Ids me_detail_residual_x_;
  MEMap3Ids me_detail_residual_y_;

  // occupancy of GEMRecHits
  MEMap1Ids me_occ_zr_;
  MEMap3Ids me_detail_occ_xy_;
  MEMap3Ids me_detail_occ_xy_ch1_; // to check GEMGeometry
  MEMap3Ids me_detail_occ_polar_;

  // occupancy of PSimHit and GEMRecHIts for efficiency
  MEMap1Ids me_simhit_occ_eta_;
  MEMap1Ids me_rechit_occ_eta_;
  MEMap2Ids me_simhit_occ_phi_;
  MEMap2Ids me_rechit_occ_phi_;
  MEMap2Ids me_simhit_occ_det_;
  MEMap2Ids me_rechit_occ_det_;
};

#endif // Validation_MuonGEMRecHits_GEMRecHitsValidation_H_
