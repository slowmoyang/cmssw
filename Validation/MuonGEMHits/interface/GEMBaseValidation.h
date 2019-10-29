#ifndef Validation_MuonGEMHits_GEMBaseValidation_H
#define Validation_MuonGEMHits_GEMBaseValidation_H

#include "DQMServices/Core/interface/DQMEDAnalyzer.h"

#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <unordered_map>

class GEMBaseValidation : public DQMEDAnalyzer {
public:
  explicit GEMBaseValidation(const edm::ParameterSet& ps);
  ~GEMBaseValidation() override;
  void analyze(const edm::Event& e, const edm::EventSetup&) override = 0;

 protected:
  const GEMGeometry* initGeometry(const edm::EventSetup &);

  // MonitorElement
  MonitorElement* bookZROccupancy(DQMStore::IBooker & ibooker,
                                  Int_t region_id,
                                  const char* name_prfix,
                                  const char* title_prefix);

  template <typename MEMapKey>
    MonitorElement* bookZROccupancy(DQMStore::IBooker & ibooker,
                                    const MEMapKey & key,
                                    const char* name_prfix,
                                    const char* title_prefix);

  template <typename MEMapKey>
    MonitorElement* bookXYOccupancy(DQMStore::IBooker & ibooker,
                                    const MEMapKey & key,
                                    const char* name_prefix,
                                    const char* title_prefix);

  template <typename MEMapKey>
    MonitorElement* bookPolarOccupancy(DQMStore::IBooker & ibooker,
                                       const MEMapKey & key,
                                       const char* name_prefix,
                                       const char* title_prefix);

  template <typename MEMapKey>
    MonitorElement* bookDetectorOccupancy(DQMStore::IBooker& ibooker,
                                          const MEMapKey & key,
                                          const GEMStation* station,
                                          const char* name_prfix,
                                          const char* title_prefix);

  template <typename MEMapKey>
    MonitorElement* bookHist1D(DQMStore::IBooker& ibooker,
                               const MEMapKey & key,
                               const char* name, const char* title,
                               Int_t nbinsx, Double_t xlow, Double_t xup,
                               const char* x_title="",
                               const char* y_title="Entries");

  template <typename MEMapKey>
    MonitorElement* bookHist2D(DQMStore::IBooker& ibooker,
                               const MEMapKey & key,
                               const char* name, const char* title,
                               Int_t nbinsx, Double_t xlow, Double_t xup,
                               Int_t nbinsy, Double_t ylow, Double_t yup,
                               const char* x_title="", const char* y_title="");

  Int_t getDetOccBinX(Int_t chamber_id, Int_t layer_id);
  Bool_t isMuonSimHit(const PSimHit &);

  //////////////////////////////////////////////////////////////////////////////
  // Parameters
  //////////////////////////////////////////////////////////////////////////////
  Int_t xy_occ_num_bins_;
  std::vector<Int_t> zr_occ_num_bins_;
  std::vector<Double_t> zr_occ_range_;
  std::vector<Double_t> eta_range_;

  std::string folder_;
  std::string log_category_;
  Bool_t detail_plot_;

  //////////////////////////////////////////////////////////////////////////////
  // Constants
  //////////////////////////////////////////////////////////////////////////////

  const Int_t kMuonPDGId_ = 13;
};

#include "Validation/MuonGEMHits/src/GEMBaseValidation.tpp"

#endif
