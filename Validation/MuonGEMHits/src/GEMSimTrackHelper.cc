#include "Validation/MuonGEMHits/interface/GEMSimTrackHelper.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "SimDataFormats/Track/interface/SimTrack.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <set>



// FIXME
void GEMSimTrackHelper::setValuesWithSimTrack(const SimTrack & sim_track)
{
  pt = sim_track.momentum().pt();
  eta = sim_track.momentum().eta();
  phi = sim_track.momentum().phi();
} 


void GEMSimTrackHelper::setValuesWithSimHitChamberIds(const std::set<unsigned int> & chamber_ids)
{
  for(auto d: chamber_ids)
  {
    const GEMDetId id(d);
    if ( id.chamber() % 2 == 0 )
      hitEven[id.station() - 1] = true;
    else if ( id.chamber() % 2 == 1 )
      hitOdd[id.station() - 1] = true;
    else
      edm::LogInfo("GEMRecHitTrackMatch") << "Error to get chamber id" << std::endl;

    gem_sh[id.station() - 1][id.layer() - 1] = true;
  }
}



void GEMSimTrackHelper::setValuesWithDigiChamberIds(const std::set<unsigned int> & chamber_ids)
{
  for(auto d: chamber_ids)
  {
    const GEMDetId id(d);
    gem_dg[id.station() - 1][id.layer() - 1] = true;
  }
}


void GEMSimTrackHelper::setValuesWithPadChamberIds(const std::set<unsigned int> & chamber_ids)
{
  for(auto d: chamber_ids)
  {
    const GEMDetId id(d);
    gem_pad[ id.station()-1][id.layer() - 1] = true;
  }
}


void GEMSimTrackHelper::setValuesWithRecHitChamberIds(const std::set<unsigned int> & chamber_ids)
{
  for(auto d: chamber_ids)
  {
    const GEMDetId id(d);
    gem_rh[ id.station() - 1][ id.layer() - 1] = true;
  }
}


void GEMSimTrackHelper::reset()
{
  pt = 0.0f;
  eta = 0.0f;
  phi = 0.0f;

  for(int s = 0; s < 3; s++)
  {
    for(int l = 0; l < 2; l++)
    {
      gem_sh[s][l] = false;
      gem_dg[s][l] = false;
      gem_pad[s][l] = false;
      gem_rh[s][l] = false;
    }

    hitOdd[s] = false;
    hitEven[s] = false;
  }
}



