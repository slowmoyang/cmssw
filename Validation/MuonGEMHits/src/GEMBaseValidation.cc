#include "Validation/MuonGEMHits/interface/GEMBaseValidation.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
//#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartitionSpecs.h"

#include <memory>

#include "TMath.h"

using namespace std;


GEMBaseValidation::GEMBaseValidation(const edm::ParameterSet& ps) {
  nBinZR_ = ps.getUntrackedParameter< std::vector<double> >("nBinGlobalZR") ;
  RangeZR_ = ps.getUntrackedParameter< std::vector<double> >("RangeGlobalZR");
  nBinXY_ = ps.getUntrackedParameter<int>("nBinGlobalXY", 360) ;
}


const GEMGeometry* GEMBaseValidation::initGeometry(edm::EventSetup const & iSetup) {
  const GEMGeometry* kGEMGeometry = nullptr;
  try {
    edm::ESHandle<GEMGeometry> hGeom;
    iSetup.get<MuonGeometryRecord>().get(hGeom);
    kGEMGeometry = &*hGeom;
  }
  catch( edm::eventsetup::NoProxyException<GEMGeometry>& e) {
    edm::LogError("MuonGEMBaseValidation") << "+++ Error : GEM geometry is unavailable on event loop. +++\n";
    return nullptr;
  }
  return kGEMGeometry;
}


GEMBaseValidation::~GEMBaseValidation() { }


MonitorElement* GEMBaseValidation::bookZROccupancy(DQMStore::IBooker& ibooker,
                                                   const char* name_prefix,
                                                   const char* title_prefix,
                                                   int region_id,
                                                   int station_id,
                                                   int layer_id) {
  const char* name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id).Data();

  TString name = TString::Format("%s_zr_occupancy%s", name_prefix, name_suffix);
  TString title = TString::Format("%s ZR Occupancy%s; globalZ[cm] ; globalR[cm]",
                                  title_prefix, title_suffix);

  // st1, st2 of xbin, st1, st2 of ybin
  int nbinsx = nBinZR_[station_id - 1];
  int nbinsy = nBinZR_[station_id + 1]; 

  // st1 xmin xmax, ymin, ymax | st2 xmin, xmax, ymin ymax
  // station1 --> 0 1 
  unsigned i = station_id == 1 ? 0 : 4;
  double xlow = RangeZR_[i];
  double xup = RangeZR_[i+1];
  double ylow = RangeZR_[i+2];
  double yup = RangeZR_[i+3];

  return ibooker.book2D(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup);
}


MonitorElement* GEMBaseValidation::bookXYOccupancy(DQMStore::IBooker& ibooker,
                                                   const char* name_prefix,
                                                   const char* title_prefix,
                                                   int region_id,
                                                   int station_id,
                                                   int layer_id) {
  const char* name_suffix  = GEMUtils::getSuffixName(region_id, station_id, layer_id).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id).Data();
  TString name = TString::Format("%s_xy_occ%s", name_prefix, name_suffix);
  TString title = TString::Format("%s XY Occupancy%s;GlobalX [cm]; GlobalY[cm]",
                                  title_prefix, title_suffix);
  return ibooker.book2D(name, title, nBinXY_, -360, 360, nBinXY_, -360, 360); 
}


MonitorElement* GEMBaseValidation::bookPolarOccupancy(DQMStore::IBooker& ibooker,
                                                      const char* name_prefix,
                                                      const char* title_prefix,
                                                      int region_id,
                                                      int station_id,
                                                      int layer_id) {

  const char* name_suffix  = GEMUtils::getSuffixName(region_id, station_id, layer_id).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id).Data();
  TString name = TString::Format("%s_polar_occ%s", name_prefix, name_suffix);
  TString title = TString::Format("%s Polar Occupancy%s", title_prefix, title_suffix);
  // TODO # of bins
  MonitorElement* me = ibooker.book2D(name, title,
                                      101, -TMath::Pi(), TMath::Pi(),
                                      101, 0.0, 2160); 
  return me;
}


MonitorElement* GEMBaseValidation::bookDetectorOccupancy(DQMStore::IBooker& ibooker,
                                                     const GEMStation* station,
                                                     const char* name_prefix,
                                                     const char* title_prefix,
                                                     int region_id) {
  int station_id = station->station();

  const char* name_suffix = GEMUtils::getSuffixName(region_id, station_id).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(region_id, station_id).Data();

  TString name = TString::Format("%s_det_occ%s", name_prefix, name_suffix); 
  TString title = TString::Format("%s Occupancy for detector component%s;;#eta-partition",
                                  title_prefix, title_suffix);

  // int nXbins = station->rings().front()->nSuperChambers()*2; //Fine for the complete geometry, but not for the Slice Test geometry
  // FIXME
  int nXbins = 72; //Maximum number of chambers is for GE1/1
  int nYbins = station->rings().front()->superChambers().front()->chambers().front()->nEtaPartitions();

  auto hist = new TH2F(title, name, nXbins, 0, nXbins, nYbins, 1, nYbins+1);

  int bin = 0 ;
  // for(unsigned sCh = 1; sCh <= station->superChambers().size(); sCh++){
  // FIXME
  for(unsigned sCh = 1; sCh <= 36; sCh++) {
    for(unsigned Ch = 1; Ch <= 2; Ch++) {
      bin++;
      TString label = TString::Format("C%dL%d", sCh, Ch);
      hist->GetXaxis()->SetBinLabel(bin, label.Data());
    }
  }

  MonitorElement* me = ibooker.book2D(name, hist);
  return me;
}  


MonitorElement* GEMBaseValidation::bookHist1D(DQMStore::IBooker& ibooker,
                                              const char* name, const char* title,
                                              int nbinsx, double xlow, double xup,
                                              int region_id,
                                              const char* x_title,
                                              const char* y_title) {
  const char* name_suffix = GEMUtils::getSuffixName(region_id);
  const char* title_suffix =GEMUtils::getSuffixTitle(region_id);
  
  TString hist_name  = TString::Format("%s%s", name, name_suffix);
  TString hist_title = TString::Format("%s :%s;%s;%s",
                                       title, title_suffix, x_title, y_title);

  return ibooker.book1D(hist_name, hist_title, nbinsx, xlow, xup);
}
    

MonitorElement* GEMBaseValidation::bookHist1D(DQMStore::IBooker& ibooker,
                                              const char* name, const char* title,
                                              int nbinsx, double xlow, double xup,
                                              int region_id, int station_id, int layer_id,
                                              const char* x_title, const char* y_title)
{
  const char* name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
  const char* title_suffix =GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
  
  TString hist_name  = TString::Format("%s%s", name, name_suffix);
  TString hist_title = TString::Format("%s :%s;%s;%s", title, title_suffix, x_title, y_title);

  return ibooker.book1D( hist_name, hist_title, nbinsx, xlow, xup);
}


MonitorElement* GEMBaseValidation::bookHist1D(DQMStore::IBooker& ibooker,
                                              const char* name, const char* title,
                                              int nbinsx, double xlow, double xup,
                                              int region_id, int station_id, int layer_id,
                                              bool is_odd_chamber,
                                              const char* x_title, const char* y_title)
{
  const char* name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
  const char* title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);

  const char* name_chamber_suffix = is_odd_chamber ? "odd" : "even";
  const char* title_chamber_suffix = is_odd_chamber ? "Odd Chambers" : "Even Chambers";
 
  TString hist_name  = TString::Format("%s%s%s", name, name_suffix, name_chamber_suffix);

  TString hist_title = TString::Format("%s :%s %s;%s;%s",   
                                       title,
                                       title_suffix, title_chamber_suffix,
                                       x_title, y_title);

  return ibooker.book1D( hist_name, hist_title, nbinsx, xlow, xup);
}



