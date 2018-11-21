#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"


#include "TString.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TEfficiency.h"

#include <tuple>


TString GEMUtils::getSuffixName(Int_t region_id) {
  return TString::Format("_re%d", region_id);
}

TString GEMUtils::getSuffixName(Int_t region_id,
                                Int_t station_id) {
  return TString::Format("_re%d_st%d", region_id, station_id);
}

TString GEMUtils::getSuffixName(Int_t region_id,
                                Int_t station_id,
                                Int_t layer_id) {
  return TString::Format("_re%d_st%d_la%d", region_id, station_id, layer_id);
}

TString GEMUtils::getSuffixName(Int_t region_id,
                                Int_t station_id,
                                Int_t layer_id,
                                Int_t roll_id) {
  return TString::Format("_re%d_st%d_la%d_ro%d",
                         region_id, station_id, layer_id, roll_id);
}

TString GEMUtils::getSuffixName(const ME2IdsKey & key) {
  Int_t region_id, station_id;
  std::tie(region_id, station_id) = key;                           
  return getSuffixName(region_id, station_id);
}

TString GEMUtils::getSuffixName(const ME3IdsKey & key) {
  Int_t region_id, station_id, layer_id;
  std::tie(region_id, station_id, layer_id) = key;
  return getSuffixName(region_id, station_id, layer_id);
}

TString GEMUtils::getSuffixName(const ME4IdsKey & key) {
  Int_t region_id, station_id, layer_id, roll_id;
  std::tie(region_id, station_id, layer_id, roll_id) = key;
  return getSuffixName(region_id, station_id, layer_id, roll_id);
}


TString GEMUtils::getSuffixTitle(Int_t region_id) {
  return TString::Format(" Region %d", region_id);
}

TString GEMUtils::getSuffixTitle(Int_t region_id,
                                 Int_t station_id) {
  return TString::Format(" Region %d Station %d", region_id, station_id);
}

TString GEMUtils::getSuffixTitle(Int_t region_id,
                                 Int_t station_id,
                                 Int_t layer_id) {
  return TString::Format(" Region %d Station %d Layer %d",
                         region_id, station_id, layer_id);
}

TString GEMUtils::getSuffixTitle(Int_t region_id,
                                 Int_t station_id,
                                 Int_t layer_id,
                                 Int_t roll_id) {
  return TString::Format(" Region %d Station %d Layer %d Roll %d",
                         region_id, station_id, layer_id, roll_id);
}

TString GEMUtils::getSuffixTitle(const ME2IdsKey & key) {
  Int_t region_id, station_id;
  std::tie(region_id, station_id) = key;                           
  return getSuffixTitle(region_id, station_id);
}


TString GEMUtils::getSuffixTitle(const ME3IdsKey & key) {
  Int_t region_id, station_id, layer_id;
  std::tie(region_id, station_id, layer_id) = key;
  return getSuffixTitle(region_id, station_id, layer_id);
}

TString GEMUtils::getSuffixTitle(const ME4IdsKey & key) {
  Int_t region_id, station_id, layer_id, roll_id;
  std::tie(region_id, station_id, layer_id, roll_id) = key;
  return getSuffixTitle(region_id, station_id, layer_id, roll_id);
}


