#ifndef VALIDATION_MUONGEMHITS_INTERFACE_GEMHITSVALIDATION_H_
#define VALIDATION_MUONGEMHITS_INTERFACE_GEMHITSVALIDATION_H_

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class GEMHitsValidation : public GEMBaseValidation
{
public:
  explicit GEMHitsValidation( const edm::ParameterSet& );
  ~GEMHitsValidation() override;
  void bookHistograms(DQMStore::IBooker &,
                      edm::Run const &,
                      edm::EventSetup const &) override;
  void analyze(const edm::Event& e, const edm::EventSetup&) override;

private:
  // r: # of regions
  // s: # of stations
  // l: # of layers

  // Simple plots
  MonitorElement* me_occ_det_[2][3]; // [r][s][l]
  MonitorElement* me_occ_zr_[2]; // [regions]

  MonitorElement* me_tof_mu_[3]; // [stations]
  MonitorElement* me_eloss_mu_[3]; // [stations]

  // Detail plots
  // [regions][stations][layers]
  MonitorElement* me_detail_occ_xy_[2][3][2];
  MonitorElement* me_detail_occ_zr_[2][3][2];
  MonitorElement* me_detail_occ_xy_chamber_[2][3][2];

  MonitorElement* me_detail_tof_[2][3][2];
  MonitorElement* me_detail_tof_mu_[2][3][2];
  MonitorElement* me_detail_eloss_[2][3][2];
  MonitorElement* me_detail_eloss_mu_[2][3][2];

  MonitorElement* me_gem_eta_phi_;


  // temporary constant
  // energy loss conversion factor
  const float kEnergyCF_ = 1.e9;


  // TODO
  edm::EDGetToken InputTagToken_;
  int nBinXY_;
  bool detailPlot_;
  std::vector<double> TOFRange_;  
  std::tuple<double, double> getTOFRange(int station_id);
};

#endif // VALIDATION_MUONGEMHITS_INTERFACE_GEMHITSVALIDATION_H_
