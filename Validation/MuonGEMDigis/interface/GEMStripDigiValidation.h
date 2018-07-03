#ifndef VALIDATION_MUONGEMDIGIS_GEMSTRIPDIGIVALIDATION_H_
#define VALIDATION_MUONGEMDIGIS_GEMSTRIPDIGIVALIDATION_H_


#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"

//#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"

class GEMStripDigiValidation : public GEMBaseValidation
{
public:
  explicit GEMStripDigiValidation(const edm::ParameterSet&);
  ~GEMStripDigiValidation() override;
  void analyze(const edm::Event& e, const edm::EventSetup&) override;
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;

private:
  // r: # of regions
  // s: # of stations
  // 

  // Simple plots
  MonitorElement* me_occ_det_[2][3]; // [r][s]
  MonitorElement* me_occ_zr_[2]; // [region]

  // Detail plots
  MonitorElement* me_detail_occ_zr_[2][3][2]; // [r]
  MonitorElement* me_detail_occ_xy_[2][3][2];
  MonitorElement* me_detail_occ_xy_chamber_[2][3][2][2];

  MonitorElement* me_detail_occ_phi_strip_[2][3][2];
  MonitorElement* me_detail_occ_strip_[2][3][2];
  MonitorElement* me_detail_bx_[2][3][2];


 
  edm::EDGetToken InputTagToken_;
  int nBinXY_;
  bool detailPlot_;
  int nStripsGE11_;
  int nStripsGE21_;
  const char* kMENamePrefix_ = "strip_digi";
};

#endif // VALIDATION_MUONGEMDIGIS_GEMSTRIPDIGIVALIDATION_H_
