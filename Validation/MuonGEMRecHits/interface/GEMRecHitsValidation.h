#ifndef VALIDATION_MUONGEMRECHITS_INTERFACE_GEMRECHITSVALIDATION_H_
#define VALIDATION_MUONGEMRECHITS_INTERFACE_GEMRECHITSVALIDATION_H_

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"

#include "DataFormats/GEMRecHit/interface/GEMRecHit.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"


class GEMRecHitsValidation : public GEMBaseValidation {
 public:
  explicit GEMRecHitsValidation(const edm::ParameterSet &);
  ~GEMRecHitsValidation() override;
  void bookHistograms(DQMStore::IBooker &,
                      edm::Run const &,
                      edm::EventSetup const &) override;
  void analyze(const edm::Event &, const edm::EventSetup &) override;

 private:
  const GEMEtaPartition * findEtaPartition(const GEMChamber* chamber,
                                           GlobalPoint & global_point);

  //Simple Plots
  MonitorElement* me_cls_;

  MEMap1Ids me_residual_x_;
  MEMap1Ids me_residual_y_;
  MEMap1Ids me_pull_x_;
  MEMap1Ids me_pull_y_;

  MEMap2Ids me_residual_x_roll_;
  MEMap2Ids me_residual_y_roll_;
  MEMap2Ids me_pull_x_roll_;
  MEMap2Ids me_pull_y_roll_;
  MEMap2Ids me_cls_roll_;

  MEMap1Ids me_rechit_occ_zr_;

  // NOTE Efficiency = matched rechit occupancy / muon occupancy
  // muon
  MEMap1Ids me_muon_occ_eta_;
  MEMap2Ids me_muon_occ_phi_;
  MEMap2Ids me_muon_occ_det_;
  // RecHit
  MEMap1Ids me_rechit_occ_eta_;
  MEMap2Ids me_rechit_occ_phi_;
  MEMap2Ids me_rechit_occ_det_;

  // Detaile Plots

  // occupancy plots for efficiency
  // first: tight <--> second: loose
  std::pair<MEMap1Ids, MEMap1Ids> me_detail_muon_occ_eta_;
  std::pair<MEMap2Ids, MEMap2Ids> me_detail_muon_occ_phi_;
  std::pair<MEMap2Ids, MEMap2Ids> me_detail_muon_occ_det_;
  std::pair<MEMap1Ids, MEMap1Ids> me_detail_rechit_occ_eta_;
  std::pair<MEMap2Ids, MEMap2Ids> me_detail_rechit_occ_phi_;
  std::pair<MEMap2Ids, MEMap2Ids> me_detail_rechit_occ_det_;

  // just occupancy plots
  MEMap3Ids me_detail_rechit_occ_xy_;
  MEMap3Ids me_detail_rechit_occ_xy_ch1_;
  MEMap3Ids me_detail_rechit_occ_zr_;
  MEMap3Ids me_detail_rechit_occ_polar_;

  // 
  MEMap3Ids me_detail_cls_;
  MEMap3Ids me_detail_residual_x_;
  MEMap3Ids me_detail_residual_y_;
  MEMap3Ids me_detail_pull_x_;
  MEMap3Ids me_detail_pull_y_;

  //  
  edm::EDGetToken rechit_token_;
  edm::EDGetTokenT<edm::View<reco::Muon> > muon_token_;
  MuonServiceProxy* muon_service_proxy_; 
  Float_t muon_pt_cut_;

  // constants
  const std::string kPropagatorName_ = "SteppingHelixPropagatorAny";
  const Int_t kTightId_ = 1, kLooseId_ = 2;

};

#endif // VALIDATION_MUONGEMRECHITS_INTERFACE_GEMRECHITSVALIDATION_H_
