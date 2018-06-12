#include "Validation/MuonGEMHits/interface/GEMTrackMatch.h"
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"

// TODO
#include "DataFormats/GeometrySurface/interface/Surface.h"
#include "DataFormats/GeometrySurface/interface/Bounds.h"

GEMTrackMatch::GEMTrackMatch(const edm::ParameterSet& ps)
{
  minPt_  = ps.getUntrackedParameter<double>("gemMinPt",5.0);
  minEta_ = ps.getUntrackedParameter<double>("gemMinEta",1.55);
  maxEta_ = ps.getUntrackedParameter<double>("gemMaxEta",2.45);
}


GEMTrackMatch::~GEMTrackMatch() {
}


void GEMTrackMatch::FillWithTrigger(MonitorElement* hist[3],
                                    Float_t eta)
{
  for( unsigned int i=0 ; i<nstation ; i++)
    hist[i]->Fill(eta);
  return;
}


void GEMTrackMatch::FillWithTrigger(MonitorElement* hist[3][3],
                                    Float_t eta,
                                    Float_t phi,
                                    bool odd[3],
                                    bool even[3])
{
  for(unsigned int i=0 ; i<nstation ; i++)
  {
    int station = i + 1;

    // Odd
    //if (odd[i]) {
    // Is it necessary to check the range of eta?
    // if (odd[i] and eta > getEtaRange(station, 1).first and eta < getEtaRange(station, 1).second ) {
    if(odd[i]) {
      hist[i][0]->Fill(phi);
      if( hist[i][1] != nullptr)
        hist[i][1]->Fill(phi);
    }

    // Even
    // if ( even[i] and eta > getEtaRange(station,2).first and eta < getEtaRange(station,2).second ) {
    if ( even[i] ) {
      hist[i][0]->Fill(phi);
      if( hist[i][2] != nullptr)
        hist[i][2]->Fill(phi);
    }
  }
  return;
}


void GEMTrackMatch::FillWithTrigger(MonitorElement* hist[4][3], bool array[3][2], Float_t value)
{
  for( unsigned int i=0 ; i<nstation ; i++)
  {
    if ( array[i][0] )                 hist[0][i]->Fill(value);
    if ( array[i][1] )                 hist[1][i]->Fill(value);
    if ( array[i][0] or array[i][1] )  hist[2][i]->Fill(value);
    if ( array[i][0] and array[i][1] ) hist[3][i]->Fill(value);
  } 
  return;
}


void GEMTrackMatch::FillWithTrigger(MonitorElement* hist[4][3][3],
                                    bool array[3][2],
                                    Float_t eta,
                                    Float_t phi,
                                    bool odd[3],
                                    bool even[3])
{
  for( unsigned int i=0 ; i<nstation ; i++)
  {
    int station = i+1;
    // if ( odd[i] and eta > getEtaRange(station,1).first and eta < getEtaRange(station,1).second ) {
    if ( odd[i] and eta ) {
      if ( array[i][0] ) { 
        hist[0][i][0]->Fill(phi);
        if ( hist[0][i][1] != nullptr) hist[0][i][1]->Fill(phi); 
      }
      if ( array[i][1] ) { 
        hist[1][i][0]->Fill(phi); 
        if( hist[1][i][1] != nullptr ) hist[1][i][1]->Fill(phi); 
      }
      if ( array[i][0] or array[i][1] ) { 
        hist[2][i][0]->Fill(phi); 
        if ( hist[2][i][1] != nullptr) hist[2][i][1]->Fill(phi); 
      }
      if ( array[i][0] and array[i][1] ) { 
        hist[3][i][0]->Fill(phi); 
        if ( hist[3][i][1] != nullptr) hist[3][i][1]->Fill(phi); 
      }
    }
    // if ( even[i] and eta > getEtaRange(station,2).first and eta < getEtaRange(station,2).second ) {
    if ( even[i] ) {
      if ( array[i][0] )
      { 
        hist[0][i][0]->Fill(phi); 
        if ( hist[0][i][2] != nullptr)
          hist[0][i][2]->Fill(phi); 
      }

      if ( array[i][1] )
      { 
        hist[1][i][0]->Fill(phi); 
        if( hist[1][i][2] != nullptr )
          hist[1][i][2]->Fill(phi); 
      }

      if ( array[i][0] or array[i][1] ) { 
        hist[2][i][0]->Fill(phi); 
        if( hist[2][i][2]!=nullptr) hist[2][i][2]->Fill(phi); 
      }
      if ( array[i][0] and array[i][1] ) { 
        hist[3][i][0]->Fill(phi); 
        if( hist[3][i][2]!=nullptr) hist[3][i][2]->Fill(phi);
      }
    }
  }
  return;
}


std::pair<double,double> GEMTrackMatch::getEtaRange(int station, int chamber)
{
  std::pair<double, double> eta_range;
  if( gem_geom_ != nullptr)
  {
    auto& ch = gem_geom_->chambers().front();
    auto& roll1 = ch->etaPartitions().front(); //.begin();
    auto& roll2 = ch->etaPartitions()[ch->nEtaPartitions() - 1];

    const BoundPlane& bSurface1(roll1->surface());
    const BoundPlane& bSurface2(roll2->surface());

    auto& parameters1( roll1->specs()->parameters());
    float height1(parameters1[2]);
    auto& parameters2( roll2->specs()->parameters());
    float height2(parameters2[2]);

    LocalPoint lTop1( 0., height1, 0.);
    GlobalPoint gTop1(bSurface1.toGlobal(lTop1));

    //LocalPoint lBottom1( 0., -height1, 0.);
    //GlobalPoint gBottom1(bSurface1.toGlobal(lBottom1));
    //LocalPoint lTop2( 0., height2, 0.);
    //GlobalPoint gTop2(bSurface2.toGlobal(lTop2));
    LocalPoint lBottom2( 0., -height2, 0.);
    GlobalPoint gBottom2(bSurface2.toGlobal(lBottom2));

    double eta1 = std::fabs(gTop1.eta()) - 0.01;
    double eta2 = std::fabs(gBottom2.eta()) + 0.01;
    eta_range = std::make_pair(eta1,eta2);
  }
  else
  {
    std::cout<<"Failed to get geometry information"<<std::endl;
    eta_range = std::make_pair(0,0);
  }
  return eta_range;
}








bool GEMTrackMatch::isSimTrackGood(const SimTrack &t)
{

  // SimTrack selection
  if (t.noVertex())   return false; 
  if (t.noGenpart()) return false;
  if (std::abs(t.type()) != 13) return false; // only interested in direct muon simtracks

  float pt = t.momentum().pt();
  // const float eta(std::fabs(t.momentum().eta()));
  // float phi = t.momentum().phi();

  if (pt < minPt_ ) return false;

  // if (eta > maxEta_ or eta < minEta_ ) return false; // no GEMs could be in such eta

  // -105 deg <= phi <= -65 deg 
  // -5 deg <= phi <= 5 deg
  // bool good_phi = (phi <= -1.13446 and phi >= -1.8326) or (phi <= 0.0872664626 and phi >= -0.0872664626);
  // if (not good_phi) return false;

  return true;
}


void GEMTrackMatch::buildLUT(const int maxChamberId)
{

  edm::LogInfo("GEMTrackMatch") << "GEMTrackMatch::buildLUT" << std::endl;
  edm::LogInfo("GEMTrackMatch") << "max chamber " << maxChamberId << std::endl;

  std::vector<int> pos_ids, neg_ids;
  std::vector<float> phis_pos;
  std::vector<float> phis_neg;
  LocalPoint lCentre( 0., 0., 0. );

  for(auto it : gem_geom_->chambers())
  {
    if(it->id().region()>0)
    {
      pos_ids.push_back(it->id().rawId());
      edm::LogInfo("GEMTrackMatch") << "added id = " << it->id() << " = " << it->id().rawId() << " to pos ids" << std::endl;
      const BoundPlane& bSurface(it->surface());
      GlobalPoint gCentre(bSurface.toGlobal(lCentre));
      int cphi(gCentre.phi().degrees());
      if (cphi < 0) cphi += 360;
      phis_pos.push_back(cphi);
      edm::LogInfo("GEMTrackMatch")<<"added phi = "<<cphi<<" to phi pos vector"<<std::endl;
    }
    else if(it->id().region()<0)
    {
      neg_ids.push_back(it->id().rawId());
      edm::LogInfo("GEMTrackMatch")<<"added id = "<<it->id()<<" = "<<it->id().rawId()<<" to neg ids"<<std::endl;
      const BoundPlane& bSurface(it->surface());
      GlobalPoint gCentre(bSurface.toGlobal(lCentre));
      int cphi(gCentre.phi().degrees());
      if (cphi < 0) cphi += 360;
      phis_neg.push_back(cphi);
      edm::LogInfo("GEMTrackMatch")<<"added phi = "<<cphi<<" to phi neg vector"<<std::endl;
    }
  }

  positiveLUT_ = std::make_pair(phis_pos, pos_ids);
  negativeLUT_ = std::make_pair(phis_neg, neg_ids);
}


void GEMTrackMatch::setGeometry(const GEMGeometry& geom)
{
  edm::LogInfo("GEMTrackMatch")<<"GEMTrackMatch :: setGeometry"<<std::endl;
  gem_geom_ = &geom;

  GEMDetId chId(gem_geom_->chambers().front()->id());
  useRoll_ = 1;

  const auto top_chamber = static_cast<const GEMEtaPartition*>(geom.idToDetUnit(GEMDetId(chId.region(),
                                                                                         chId.ring(),
                                                                                         chId.station(),
                                                                                         chId.layer(),
                                                                                         chId.chamber(),
                                                                                         useRoll_)));

  const int nEtaPartitions(gem_geom_->chambers().front()->nEtaPartitions());

  const auto bottom_chamber = static_cast<const GEMEtaPartition*>(geom.idToDetUnit(GEMDetId(chId.region(),chId.ring(),chId.station(),chId.layer(),chId.chamber(), nEtaPartitions)));

  const float top_half_striplength = top_chamber->specs()->specificTopology().stripLength() / 2.;
  const float bottom_half_striplength = bottom_chamber->specs()->specificTopology().stripLength() / 2.;

  const LocalPoint lp_top(0., top_half_striplength, 0.);
  const LocalPoint lp_bottom(0., -bottom_half_striplength, 0.);

  const GlobalPoint gp_top = top_chamber->toGlobal(lp_top);
  const GlobalPoint gp_bottom = bottom_chamber->toGlobal(lp_bottom);

  radiusCenter_ = (gp_bottom.perp() + gp_top.perp())/2.;
  chamberHeight_ = gp_top.perp() - gp_bottom.perp();

  const int maxChamberId = geom.regions()[0]->stations()[0]->superChambers().size();

  edm::LogInfo("GEMTrackMatch")<<"GEMTrackMatch :: setGeometry --> Calling buildLUT"<<std::endl;

  buildLUT(maxChamberId);

  // TODO
  // a vcector of bounds
  // for(auto chamber : gem_geom_->chambers())
  // {
  //   const Plane & chamber_surface = chamber->surface();
  //  const Bounds& chamber_bounds = chamber_surface.bounds();
  // }



}  


std::pair<int,int> GEMTrackMatch::getClosestChambers(const int maxChamberId , int region, float phi)
{
  auto& phis(positiveLUT_.first);
  auto upper = std::upper_bound(phis.begin(), phis.end(), phi);
  auto& LUT = (region == 1 ? positiveLUT_.second : negativeLUT_.second);
  return std::make_pair(LUT.at(upper - phis.begin()), (LUT.at((upper - phis.begin() + 1)%maxChamberId)));
}


std::pair<double, double> GEMTrackMatch::getEtaRangeForPhi( int station ) 
{
  std::pair<double, double> range;
  // TODO
  // there are no lines that initialize etaRangeForPhi
  if( station== 0 )      range = std::make_pair( etaRangeForPhi[0],etaRangeForPhi[1]) ; 
  else if( station== 1 ) range = std::make_pair( etaRangeForPhi[2],etaRangeForPhi[3]) ; 
  else if( station== 2 ) range = std::make_pair( etaRangeForPhi[4],etaRangeForPhi[5]) ; 

  return range;
}

