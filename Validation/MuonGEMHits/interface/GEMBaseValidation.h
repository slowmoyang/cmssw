#ifndef VALIDATION_MUONGEMHITS_INTERFACE_GEMBASEVALIDATION_H_
#define VALIDATION_MUONGEMHITS_INTERFACE_GEMBASEVALIDATION_H_

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"

#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

class GEMBaseValidation : public DQMEDAnalyzer
{
public:
  explicit GEMBaseValidation(const edm::ParameterSet& ps);
  ~GEMBaseValidation() override;
  void analyze(const edm::Event& e, const edm::EventSetup&) override = 0 ;

protected:
  const GEMGeometry* initGeometry(const edm::EventSetup&);

  // MonitorElement
  MonitorElement* bookZROccupancy(DQMStore::IBooker & ibooker,
                                  const char* name_prfix,
                                  const char* title_prefix,
                                  int region_id,
                                  int station_id=999,
                                  int layer_id=999);

  MonitorElement* bookXYOccupancy(DQMStore::IBooker & ibooker,
                                  const char* name_prefix,
                                  const char* title_prefix,
                                  int region_id,
                                  int station_id=999,
                                  int layer_id=999);

  MonitorElement* bookPolarOccupancy(DQMStore::IBooker & ibooker,
                                     const char* name_prefix,
                                     const char* title_prefix,
                                     int region_id,
                                     int station_id,
                                     int layer_id);

  MonitorElement* bookDetectorOccupancy(DQMStore::IBooker& ibooker,
                                  const GEMStation* station,
                                  const char* name_prfix,
                                  const char* title_prefix,
                                  int region_id);

  MonitorElement* bookHist1D(DQMStore::IBooker& ibooker,
                             const char* name, const char* title,
                             int nbinsx, double xlow, double xup,
                             int region_id,
                             const char* x_title="", const char* y_title="");

  MonitorElement* bookHist1D(DQMStore::IBooker& ibooker,
                             const char* name, const char* title,
                             int nbinsx, double xlow, double xup,
                             int region_id, int station_id, int layer_id,
                             const char* x_title="", const char* y_title="");

  MonitorElement* bookHist1D(DQMStore::IBooker& ibooker,
                             const char* name, const char* title,
                             int nbinsx, double xlow, double xup,
                             int region_id, int station_id, int layer_id,
                             bool is_odd_chamber,
                             const char* x_title="", const char* y_title="");

  inline unsigned getRegionIndex(int region_id) {
    return static_cast<unsigned>(region_id == -1 ? 0 : 1);
  }

  inline unsigned getStationIndex(int station_id) {
    return static_cast<unsigned>(station_id - 1);
  }

  inline unsigned getLayerIndex(int layer_id) {
    return static_cast<unsigned>(layer_id - 1);
  }

  // Parameters
  int nBinXY_;
  std::vector<double> nBinZR_;
  std::vector<double> RangeZR_;

  // Cosntants
  int kMuonPDGId_ = 13;


private :
  std::vector<TString> region_label_;
  std::vector<TString> layer_label_;

  edm::EDGetToken InputTagToken_;

  bool detailPlot_;

};

#endif // VALIDATION_MUONGEMHITS_INTERFACE_GEMBASEVALIDATION_H_
