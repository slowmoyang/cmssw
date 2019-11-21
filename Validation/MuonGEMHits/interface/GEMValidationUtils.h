#ifndef Validation_MuonGEMHits_INTERFACE_GEMValidationUtils_h_
#define Validation_MuonGEMHits_INTERFACE_GEMValidationUtils_h_

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "TString.h"
#include "TSystem.h"

#include <map>
#include <tuple>

using namespace dqm::impl;

// class MonitorElement;
class TH1F;
class TH2F;
class TProfile;

using ME2IdsKey = std::tuple<Int_t, Int_t>;
using ME3IdsKey = std::tuple<Int_t, Int_t, Int_t>;
using ME4IdsKey = std::tuple<Int_t, Int_t, Int_t, Int_t>;

using MEMap1Ids = std::map<Int_t, MonitorElement*>;
using MEMap2Ids = std::map<ME2IdsKey, MonitorElement*>;
using MEMap3Ids = std::map<ME3IdsKey, MonitorElement*>;
using MEMap4Ids = std::map<ME4IdsKey, MonitorElement*>;

namespace GEMUtils {

TString getSuffixName(Int_t region_id);
TString getSuffixName(Int_t region_id, Int_t station_id);
TString getSuffixName(Int_t region_id, Int_t station_id, Int_t layer_id);
TString getSuffixName(Int_t region_id, Int_t station_id,
                      Int_t layer_id, Int_t roll_id);

TString getSuffixName(const ME2IdsKey & key);
TString getSuffixName(const ME3IdsKey & key);
TString getSuffixName(const ME4IdsKey & key);

TString getSuffixTitle(Int_t region_id);
TString getSuffixTitle(Int_t region_id, Int_t station_id);
TString getSuffixTitle(Int_t region_id, Int_t station_id, Int_t layer_id);
TString getSuffixTitle(Int_t region_id, Int_t station_id,
                       Int_t layer_id, Int_t roll_id);

TString getSuffixTitle(const ME2IdsKey & key);
TString getSuffixTitle(const ME3IdsKey & key);
TString getSuffixTitle(const ME4IdsKey & key);

}


#endif // Validation_MuonGEMHits_GEMValidationUtils_h_
