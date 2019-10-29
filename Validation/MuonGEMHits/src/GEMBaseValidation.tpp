#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"


#include "TMath.h"

template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookZROccupancy(DQMStore::IBooker& ibooker,
                                                   const MEMapKey & key,
                                                   const char* name_prefix,
                                                   const char* title_prefix) {
  // TODO Logging
  if (std::tuple_size<MEMapKey>::value < 2) return nullptr;

  Int_t station_id = std::get<1>(key);

  const char* name_suffix = GEMUtils::getSuffixName(key).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(key).Data();

  TString name = TString::Format("%s_occ_zr%s", name_prefix, name_suffix);
  TString title = TString::Format("%s ZR Occupancy :%s;|Z| #[cm];R [cm]",
                                  title_prefix, title_suffix);

  // NOTE currently, only GE11 and GE21 are considered.
  // Look Validation/MuonGEMHits/python/MuonGEMCommonParameters_cfi.py
  UInt_t nbins_start = 2 * (station_id - 1);
  Int_t nbinsx = zr_occ_num_bins_[nbins_start];
  Int_t nbinsy = zr_occ_num_bins_[nbins_start + 1];

  // st1 xmin xmax, ymin, ymax | st2 xmin, xmax, ymin ymax
  UInt_t range_start = 4 * (station_id - 1);
  // absolute z axis
  Double_t xlow = zr_occ_range_[range_start];
  Double_t xup = zr_occ_range_[range_start + 1];
  // R axis
  Double_t ylow = zr_occ_range_[range_start + 2];
  Double_t yup = zr_occ_range_[range_start + 3];

  return ibooker.book2D(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup);
}


template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookXYOccupancy(DQMStore::IBooker& ibooker,
                                                   const MEMapKey & key,
                                                   const char* name_prefix,
                                                   const char* title_prefix) {
  const char* name_suffix  = GEMUtils::getSuffixName(key);
  const char* title_suffix = GEMUtils::getSuffixTitle(key);
  TString name = TString::Format("%s_occ_xy%s", name_prefix, name_suffix);
  TString title = TString::Format("%s XY Occupancy :%s;X [cm];Y [cm]",
                                  title_prefix, title_suffix);
  return ibooker.book2D(name, title,
                        xy_occ_num_bins_, -360, 360,
                        xy_occ_num_bins_, -360, 360);
}


template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookPolarOccupancy(
    DQMStore::IBooker& ibooker,
    const MEMapKey & key,
    const char* name_prefix,
    const char* title_prefix) {

  const char* name_suffix  = GEMUtils::getSuffixName(key);
  const char* title_suffix = GEMUtils::getSuffixTitle(key);
  TString name = TString::Format("%s_occ_polar%s", name_prefix, name_suffix);
  TString title = TString::Format("%s Polar Occupancy :%s",
                                  title_prefix, title_suffix);
  // TODO # of bins
  // TODO the x-axis lies in the cnter of Ch1
  MonitorElement* me = ibooker.book2D(name, title,
                                      108, -M_PI, M_PI,
                                      108, 0.0f, 360.0f);
  return me;
}


template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookDetectorOccupancy(
    DQMStore::IBooker& ibooker,
    const MEMapKey & key,
    const GEMStation* station,
    const char* name_prefix,
    const char* title_prefix) {

  const char* name_suffix = GEMUtils::getSuffixName(key).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(key).Data();

  TString name = TString::Format("%s_occ_det%s", name_prefix, name_suffix);
  TString title = TString::Format("%s Occupancy for detector component :%s",
      title_prefix, title_suffix);

  std::vector<const GEMSuperChamber*> superchambers = station->superChambers();

  Int_t num_superchambers = superchambers.size();
  Int_t num_chambers = superchambers.front()->nChambers();

  Int_t nbinsx = num_superchambers * num_chambers;
  Int_t nbinsy = superchambers.front()->chambers().front()->nEtaPartitions();

  auto hist = new TH2F(name, title,
                       nbinsx, 1 - 0.5, nbinsx + 0.5,
                       nbinsy, 1 - 0.5, nbinsy + 0.5);
  hist->GetXaxis()->SetTitle("Chamber-Layer");
  hist->GetYaxis()->SetTitle("Eta Partition");

  TAxis* x_axis = hist->GetXaxis();
  for (Int_t chamber_id = 1; chamber_id <= num_superchambers; chamber_id++) {
    for (Int_t layer_id = 1; layer_id <= num_chambers; layer_id++) {
      Int_t bin = getDetOccBinX(chamber_id, layer_id);
      TString label = TString::Format("C%dL%d", chamber_id, layer_id);
      x_axis->SetBinLabel(bin, label);
    }
  }

  TAxis* y_axis = hist->GetYaxis();
  for (Int_t bin = 1; bin <= nbinsy; bin++) {
    y_axis->SetBinLabel(bin, TString::Itoa(bin, 10));
  }

  return ibooker.book2D(name, hist);
}



template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookHist1D(DQMStore::IBooker& ibooker,
                                              const MEMapKey & key,
                                              const char* name,
                                              const char* title,
                                              Int_t nbinsx,
                                              Double_t xlow,
                                              Double_t xup,
                                              const char* x_title,
                                              const char* y_title) {
  const char* name_suffix = GEMUtils::getSuffixName(key);
  const char* title_suffix = GEMUtils::getSuffixTitle(key);

  TString hist_name  = TString::Format("%s%s", name, name_suffix);
  TString hist_title = TString::Format("%s :%s;%s;%s",
                                       title, title_suffix, x_title, y_title);

  return ibooker.book1D(hist_name, hist_title, nbinsx, xlow, xup);
}


template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookHist2D(
    DQMStore::IBooker& ibooker,
    const MEMapKey & key,
    const char* name, const char* title,
    Int_t nbinsx, Double_t xlow, Double_t xup,
    Int_t nbinsy, Double_t ylow, Double_t yup,
    const char* x_title, const char* y_title) {

  const char* name_suffix = GEMUtils::getSuffixName(key);
  const char* title_suffix = GEMUtils::getSuffixTitle(key);

  TString hist_name  = TString::Format("%s%s", name, name_suffix);
  TString hist_title = TString::Format("%s :%s;%s;%s",
                                       title, title_suffix, x_title, y_title);

  return ibooker.book2D(hist_name, hist_title,
                        nbinsx, xlow, xup,
                        nbinsy, ylow, yup);
}
