#ifndef VALIDATION_MUONGEMHITS_INTERFACE_GEMVALIDATIONUTILS_H_
#define VALIDATION_MUONGEMHITS_INTERFACE_GEMVALIDATIONUTILS_H_

#include "TString.h"

namespace GEMUtils {
  TString getSuffixName(int region_id);
  TString getSuffixName(int region_id, int station_id);
  TString getSuffixName(int region_id, int station_id, int layer_id);
  TString getSuffixName(int region_id, int station_id, int layer_id, bool is_odd_chamber);

  TString getSuffixTitle(int region_id);
  TString getSuffixTitle(int region_id, int station_id);
  TString getSuffixTitle(int region_id, int station_id, int layer_id);
  TString getSuffixTitle(int region_id, int station_id, int layer_id, bool is_odd_chamber);
}
#endif // VALIDATION_MUONGEMHITS_GEMVALIDATIONUTILS_H_
