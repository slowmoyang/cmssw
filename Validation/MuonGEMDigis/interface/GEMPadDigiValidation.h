#ifndef VALIDATION_MUONGEMDIGIS_INTERFACE_GEMPADDIGIVALIDATION_H_
#define VALIDATION_MUONGEMDIGIS_INTERFACE_GEMPADDIGIVALIDATION_H_

#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"
#include "DataFormats/GEMDigi/interface/GEMPadDigiCollection.h"

//#include "DataFormats/Common/interface/Handle.h"
class GEMPadDigiValidation : public GEMBaseValidation
{
public:
  explicit GEMPadDigiValidation( const edm::ParameterSet& );
  ~GEMPadDigiValidation() override;
  void analyze(const edm::Event& e, const edm::EventSetup&) override;
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
 private:
  // Simple plots
  MonitorElement* me_occ_det_[2][3];
  MonitorElement* me_occ_zr_[2];

  // Detail plots
  MonitorElement* me_detail_occ_zr_[2][3][2];
  MonitorElement* me_detail_occ_xy_[2][3][2]; 
  MonitorElement* me_detail_occ_xy_chamber_[2][3][2][2]; // 

  MonitorElement* me_detail_occ_phi_pad_[2][3][2];
  MonitorElement* me_detail_occ_pad_[2][3][2]; // DIGI Occupancy per Pad number
  MonitorElement* me_detail_bx_[2][3][2];


  edm::EDGetToken InputTagToken_;
  int nBinXY_;
  bool detailPlot_;
};

#endif // VALIDATION_MUONGEMDIGIS_GEMPADDIGIVALIDATION_H_
