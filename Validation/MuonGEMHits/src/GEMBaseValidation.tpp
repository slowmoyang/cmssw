#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"
#include "TMath.h"

template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookZROccupancy(DQMStore::IBooker& ibooker,
                                                   const MEMapKey & key,
                                                   const char* name_prefix,
                                                   const char* title_prefix) {
  // TODO Logging
  const unsigned long key_size = std::tuple_size<MEMapKey>::value;
  if(key_size < 2) {
    MonitorElement* me = nullptr;
    return me;
  }

  Int_t station_id = std::get<1>(key);

  const char* name_suffix = GEMUtils::getSuffixName(key).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(key).Data();

  TString name = TString::Format("%s_occ_zr%s", name_prefix, name_suffix);
  TString title = TString::Format("%s ZR Occupancy%s; |Z| [cm] ; R [cm]",
                                  title_prefix, title_suffix);

  // st1, st2 of xbin, st1, st2 of ybin
  Int_t nbinsx = nBinZR_[station_id - 1];
  Int_t nbinsy = nBinZR_[station_id + 1]; 

  // st1 xmin xmax, ymin, ymax | st2 xmin, xmax, ymin ymax
  // station1 --> 0 1 
  // start index
  unsigned i = station_id == 1 ? 0 : 4;
  // z
  Double_t xlow = RangeZR_[i];
  Double_t xup = RangeZR_[i+1];
  // r
  Double_t ylow = RangeZR_[i+2];
  Double_t yup = RangeZR_[i+3];

  return ibooker.book2D(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup);
}


template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookXYOccupancy(DQMStore::IBooker& ibooker,
                                                   const MEMapKey & key,
                                                   const char* name_prefix,
                                                   const char* title_prefix) {
  const char* name_suffix  = GEMUtils::getSuffixName(key).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(key).Data();
  TString name = TString::Format("%s_occ_xy%s", name_prefix, name_suffix);
  TString title = TString::Format("%s XY Occupancy%s;X [cm];Y [cm]",
                                  title_prefix, title_suffix);
  return ibooker.book2D(name, title, nBinXY_, -360, 360, nBinXY_, -360, 360); 
}


template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookXYOccupancy(DQMStore::IBooker& ibooker,
                                                   const MEMapKey & key,
                                                   const char* name_prefix,
                                                   const char* title_prefix,
                                                   const char* extra_name_suffix,
                                                   const char* extra_title_suffix) {
  const char* name_suffix  = GEMUtils::getSuffixName(key).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(key).Data();
  TString name = TString::Format("%s_occ_xy%s_%s", name_prefix, name_suffix, extra_name_suffix);
  TString title = TString::Format("%s XY Occupancy%s %s;X [cm];Y [cm]",
                                  title_prefix, title_suffix, extra_title_suffix);
  return ibooker.book2D(name, title, nBinXY_, -360, 360, nBinXY_, -360, 360); 
}



template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookPolarOccupancy(DQMStore::IBooker& ibooker,
                                                      const MEMapKey & key,
                                                      const char* name_prefix,
                                                      const char* title_prefix) {
  const char* name_suffix  = GEMUtils::getSuffixName(key).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(key).Data();
  TString name = TString::Format("%s_occ_polar%s", name_prefix, name_suffix);
  TString title = TString::Format("%s Polar Occupancy%s", title_prefix, title_suffix);
  // TODO # of bins
  MonitorElement* me = ibooker.book2D(name, title,
                                      101, -TMath::Pi(), TMath::Pi(),
                                      101, 0.0, 2160); 
  return me;
}


template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookDetectorOccupancy(DQMStore::IBooker& ibooker,
                                                         const MEMapKey & key,
                                                         const GEMStation* station,
                                                         const char* name_prefix,
                                                         const char* title_prefix) {

  const char* name_suffix = GEMUtils::getSuffixName(key).Data();
  const char* title_suffix = GEMUtils::getSuffixTitle(key).Data();

  TString name = TString::Format("%s_occ_det%s", name_prefix, name_suffix); 
  TString title = TString::Format("%s Occupancy for detector component%s;;#eta-partition",
                                  title_prefix, title_suffix);

  // previous comment
  // Fine for the complete geometry, but not for the Slice Test geometry
  // int nXbins = station->rings().front()->nSuperChambers()*2;
  // FIXME don't use  hardcoded number

  Int_t station_id = station->station();

  Int_t nbinsx = 0;
  if(station_id == 1) {
    //Maximum number of chambers is for GE1/1
    nbinsx = 72;
  } else if (station_id == 2) {
    nbinsx = 36;
  } else {
    // TODO kLogCategory to protected GEMBaseValidation attributes
    edm::LogError("GEMBaseValidation") << station_id << " case are not implemented\n";
  }

  // Int_t nYbins = station->rings().front()->superChambers().front()->chambers().front()->nEtaPartitions();
  Int_t nbinsy = 8;

  auto hist = new TH2F(title, name,
                       nbinsx, 1 - 0.5, nbinsx + 0.5,
                       nbinsy, 1 - 0.5, nbinsy + 0.5);

  // for(unsigned sCh = 1; sCh <= station->superChambers().size(); sCh++){
  // FIXME
  for(Int_t chamber_id = 1; chamber_id <= static_cast<Int_t>(nbinsx/2); chamber_id++) {
    for(Int_t layer_id = layer_id; layer_id <= 2; layer_id++) {
      Int_t bin = getDetOccBinX(chamber_id, layer_id); 
      TString label = TString::Format("C%dL%d", chamber_id, layer_id);
      hist->GetXaxis()->SetBinLabel(bin, label.Data());
    }
  }

  MonitorElement* me = ibooker.book2D(name, hist);
  return me;
}  



template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookHist1D(DQMStore::IBooker& ibooker,
                                              const MEMapKey & key,
                                              const char* name, const char* title,
                                              Int_t nbinsx, Double_t xlow, Double_t xup,
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
MonitorElement* GEMBaseValidation::bookHist1D(DQMStore::IBooker& ibooker,
                                              const MEMapKey & key,
                                              const char* name, const char* title,
                                              Int_t xlow, Int_t xup,
                                              const char* x_title,
                                              const char* y_title) {
  // NOTE it cotains xup

  const char* name_suffix = GEMUtils::getSuffixName(key);
  const char* title_suffix = GEMUtils::getSuffixTitle(key);
  
  TString hist_name  = TString::Format("%s%s", name, name_suffix);
  TString hist_title = TString::Format("%s :%s;%s;%s",
                                       title, title_suffix, x_title, y_title);
  Int_t nbinsx = xup - xlow + 1;

  TH1F* hist = new TH1F(hist_name, hist_title, nbinsx, xlow - 0.5, xup + 0.5);

  TAxis* x_axis = hist->GetXaxis();
  for(Int_t x = xlow; x <= xup; x++) {
    Int_t bin = x_axis->FindBin(x);
    TString label = TString::Format("%d", x);
    x_axis->SetBinLabel(bin, label);
  }

  return ibooker.book1D(hist_name.Data(), hist);
}

   

template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookHist2D(DQMStore::IBooker& ibooker,
                                              const MEMapKey & key,
                                              const char* name, const char* title,
                                              Int_t nbinsx, Double_t xlow, Double_t xup,
                                              Int_t nbinsy, Double_t ylow, Double_t yup,
                                              const char* x_title,
                                              const char* y_title) {
  const char* name_suffix = GEMUtils::getSuffixName(key);
  const char* title_suffix = GEMUtils::getSuffixTitle(key);
  
  TString hist_name  = TString::Format("%s%s", name, name_suffix);
  TString hist_title = TString::Format("%s :%s;%s;%s",
                                       title, title_suffix, x_title, y_title);

  return ibooker.book2D(hist_name, hist_title, nbinsx, xlow, xup, nbinsy, ylow, yup);
}


template <typename MEMapKey>
MonitorElement* GEMBaseValidation::bookHist2D(DQMStore::IBooker& ibooker,
                                              const MEMapKey & key,
                                              const char* name, const char* title,
                                              Int_t xlow, Int_t xup,
                                              Int_t ylow, Int_t yup,
                                              const char* x_title,
                                              const char* y_title) {
  // NOTE it cotains xup

  const char* name_suffix = GEMUtils::getSuffixName(key);
  const char* title_suffix = GEMUtils::getSuffixTitle(key);
  
  TString hist_name  = TString::Format("%s%s", name, name_suffix);
  TString hist_title = TString::Format("%s :%s;%s;%s",
                                       title, title_suffix, x_title, y_title);
  Int_t nbinsx = xup - xlow + 1;
  Int_t nbinsy = yup - ylow + 1;

  TH2F* hist = new TH2F(hist_name, hist_title,
                        nbinsx, xlow - 0.5, xup + 0.5,
                        nbinsy, ylow - 0.5, yup + 0.5);

  TAxis* x_axis = hist->GetXaxis();
  for(Int_t x = xlow; x <= xup; x++) {
    Int_t bin = x_axis->FindBin(x);
    TString label = TString::Format("%d", x);
    x_axis->SetBinLabel(bin, label);
  }

  TAxis* y_axis = hist->GetYaxis();
  for(Int_t y = ylow; y <= yup; y++) {
    Int_t bin = y_axis->FindBin(y);
    TString label = TString::Format("%d", y);
    y_axis->SetBinLabel(bin, label);
  }


  return ibooker.book2D(hist_name.Data(), hist);
}

