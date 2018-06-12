#ifndef Validation_MuonGEMHits_GEMSimTrackHelper_H
#define Validation_MuonGEMHits_GEMSimTrackHelper_H

#include "SimDataFormats/Track/interface/SimTrack.h"

#include <set>

class GEMSimTrackHelper
{
public:
  Float_t pt, eta, phi;
  Char_t gem_sh_layer1, gem_sh_layer2;
  Char_t gem_dg_layer1, gem_dg_layer2;
  Char_t gem_pad_layer1, gem_pad_layer2;
  Char_t has_gem_dg_l1, has_gem_dg_l2;
  Char_t has_gem_pad_l1, has_gem_pad_l2;
  Char_t has_gem_sh_l1, has_gem_sh_l2;
  // 3: # of stations
  // 2: # of layers
  bool gem_sh[3][2];
  bool gem_dg[3][2] ;
  bool gem_pad[3][2] ;
  bool gem_rh[3][2] ;
  bool hitOdd[3];
  bool hitEven[3];

  void reset();
  void setValuesWithSimTrack(const SimTrack & sim_track);
  void setValuesWithSimHitChamberIds(const std::set<unsigned int> & chamber_ids);
  void setValuesWithDigiChamberIds(const std::set<unsigned int> & chamber_ids);
  void setValuesWithPadChamberIds(const std::set<unsigned int> & chamber_ids);
  void setValuesWithRecHitChamberIds(const std::set<unsigned int> & chamber_ids);
};

#endif

