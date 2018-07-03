#ifndef VALIDATION_MUONGEMDIGIS_GEMCOPADDIGIVALIDATION_H_
#define VALIDATION_MUONGEMDIGIS_GEMCOPADDIGIVALIDATION_H_

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"
#include "DataFormats/GEMDigi/interface/GEMCoPadDigiCollection.h"


class GEMCoPadDigiValidation : public GEMBaseValidation
{
public:
  explicit GEMCoPadDigiValidation( const edm::ParameterSet& );
  ~GEMCoPadDigiValidation() override;
  void analyze(const edm::Event& e, const edm::EventSetup&) override;
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;

private:
  // Simple plots
  MonitorElement* me_occ_zr_[2]; // [# of regions]
  MonitorElement* me_occ_det_[2][3]; // [# of regions][# of statons]


  // DetailPlots
  MonitorElement* me_detail_occ_xy_[2][3];
  MonitorElement* me_detail_occ_zr_[2][3];
  MonitorElement* me_detail_occ_xy_chamber_[2][3][2];

  MonitorElement* me_detail_occ_phi_pad_[2][3];
  MonitorElement* me_detail_occ_pad_[2][3];
  MonitorElement* me_detail_bx_[2][3];

  int minBXGEM_, maxBXGEM_;
  edm::EDGetToken InputTagToken_;
  int nBinXY_;
  bool detailPlot_;
};

#endif // VALIDATION_MUONGEMDIGIS_GEMCOPADDIGIVALIDATION_H_
