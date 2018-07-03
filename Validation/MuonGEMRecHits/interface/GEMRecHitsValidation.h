#ifndef VALIDATION_MUONGEMRECHITS_INTERFACE_GEMRECHITSVALIDATION_H_
#define VALIDATION_MUONGEMRECHITS_INTERFACE_GEMRECHITSVALIDATION_H_

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"

#include <DataFormats/GEMRecHit/interface/GEMRecHit.h>
#include <DataFormats/GEMRecHit/interface/GEMRecHitCollection.h>

class GEMRecHitsValidation : public GEMBaseValidation
{
public:
  explicit GEMRecHitsValidation( const edm::ParameterSet& );
  ~GEMRecHitsValidation() override;
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
  void analyze(const edm::Event& e, const edm::EventSetup&) override;

private:
  //Simple Plots
  MonitorElement* me_occ_zr_[2]; // [# of region]
  MonitorElement* me_occ_det_[2][3]; // [# of region][# of station]

  MonitorElement* me_cls_;
  MonitorElement* me_pull_x_[2]; // [# of region]
  MonitorElement* me_pull_y_[2];

  //Detaile Plots
  // foo[# of regions][# of stations][# of layers] 
  MonitorElement* me_detail_occ_xy_[2][3][2];
  MonitorElement* me_detail_occ_zr_[2][3][2];
  MonitorElement* me_detail_occ_polar_[2][3][2];

  MonitorElement* me_detail_cls_[2][3][2];
  MonitorElement* me_detail_pull_x_[2][3][2];
  MonitorElement* me_detail_pull_y_[2][3][2];

  // moved from GEMRecHitTrackMatch
  // [# of regions][# of stations][# of layers][chamber even or odd]
  MonitorElement* me_detail_sim_occ_eta_[2][3][2][2];
  MonitorElement* me_detail_rec_occ_eta_[2][3][2][2]; // matched rechit
  MonitorElement* me_detail_sim_occ_phi_[2][3][2][2];
  MonitorElement* me_detail_rec_occ_phi_[2][3][2][2];



  MonitorElement* me_eta_phi_;

  edm::EDGetToken InputTagToken_, InputTagToken_RH;
  int nBinXY_;
  bool detailPlot_;
  std::string folder_;
};

#endif // VALIDATION_MUONGEMRECHITS_INTERFACE_GEMRECHITSVALIDATION_H_
