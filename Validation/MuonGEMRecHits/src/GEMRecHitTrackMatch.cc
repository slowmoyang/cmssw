#include "Validation/MuonGEMRecHits/interface/GEMRecHitTrackMatch.h"
#include "Validation/MuonGEMHits/interface/GEMDetLabel.h"
#include "Validation/MuonGEMHits/interface/GEMSimTrackHelper.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include <DataFormats/GEMRecHit/interface/GEMRecHit.h>
#include <DataFormats/GEMRecHit/interface/GEMRecHitCollection.h>
#include "DQMServices/Core/interface/DQMStore.h"

#include <TMath.h>
#include <TH1F.h>

#include <utility> // make_pair

using namespace std;
GEMRecHitTrackMatch::GEMRecHitTrackMatch(const edm::ParameterSet& ps) : GEMTrackMatch(ps)
{
  std::string simInputLabel_ = ps.getUntrackedParameter<std::string>("simInputLabel");
  simHitsToken_ = consumes<edm::PSimHitContainer>(edm::InputTag(simInputLabel_,"MuonGEMHits"));
  simTracksToken_ = consumes< edm::SimTrackContainer >(ps.getParameter<edm::InputTag>("simTrackCollection"));
  simVerticesToken_ = consumes< edm::SimVertexContainer >(ps.getParameter<edm::InputTag>("simVertexCollection"));

  gem_recHitToken_ = consumes<GEMRecHitCollection>(ps.getParameter<edm::InputTag>("gemRecHitInput"));

  detailPlot_ = ps.getParameter<bool>("detailPlot");
  cfg_ = ps;
}

void GEMRecHitTrackMatch::bookHistograms(DQMStore::IBooker& ibooker,
                                         edm::Run const& run,
                                         edm::EventSetup const & iSetup)
{
  edm::LogInfo("GEMRecHitTrackMatch")<<"GEM RecHitTrackMatch :: bookHistograms"<<std::endl;
  edm::ESHandle<GEMGeometry> hGeom;
  iSetup.get<MuonGeometryRecord>().get(hGeom);
  const GEMGeometry& kGEMGeom = *hGeom;
  edm::LogInfo("GEMRecHitTrackMatch")<<"GEM RecHitTrackMatch :: about to set the geometry"<<std::endl;
  setGeometry(kGEMGeom);
  edm::LogInfo("GEMRecHitTrackMatch")<<"GEM RecHitTrackMatch :: successfully set the geometry"<<std::endl;
  edm::LogInfo("GEMRecHitTrackMatch")<<"GEM RecHitTrackMatch :: geom = "<< &kGEMGeom << std::endl;
    
  ibooker.setCurrentFolder("MuonGEMRecHitsV/GEMRecHitsTask");
  edm::LogInfo("GEMRecHitTrackMatch")<<"ibooker set current folder\n";
    
  const float PI=TMath::Pi();

  using namespace GEMDetLabel;

  nstation = kGEMGeom.regions()[0]->stations().size(); 
  for( unsigned int j=0 ; j<nstation ; j++) {
    string track_eta_name  = string("track_eta")+s_suffix.at(j);
    string track_eta_title = string("track_eta")+";SimTrack |#eta|;# of tracks";
    // track_eta[j] = ibooker.book1D(track_eta_name.c_str(), track_eta_title.c_str(),140,minEta_,maxEta_);
    me_track_eta_[j] = ibooker.book1D(track_eta_name.c_str(), track_eta_title.c_str(),101,-3, 3);

    for ( unsigned int k = 0 ; k < c_suffix.size() ; k++) {
      string suffix = string(s_suffix[j])+ string(c_suffix[k]);
      string track_phi_name  = string("track_phi")+suffix;
      string track_phi_title = string("track_phi")+suffix+";SimTrack #phi;# of tracks";
      me_track_phi_[j][k] = ibooker.book1D(track_phi_name.c_str(), track_phi_title.c_str(),200,-PI,PI);
    }


    for( unsigned int i=0 ; i < l_suffix.size(); i++) {
      string suffix = string(s_suffix[j])+string(l_suffix[i]);
      string rh_eta_name = string("rh_eta")+suffix;
      string rh_eta_title = rh_eta_name+"; tracks |#eta|; # of tracks";
      // rh_eta[i][j] = ibooker.book1D( rh_eta_name.c_str(), rh_eta_title.c_str(), 140, minEta_, maxEta_) ;
      me_rh_eta_[i][j] = ibooker.book1D( rh_eta_name.c_str(), rh_eta_title.c_str(), 101, -3, 3) ;

      string rh_sh_eta_name = string("rh_sh_eta")+suffix;
      string rh_sh_eta_title = rh_sh_eta_name+"; tracks |#eta|; # of tracks";
      // rh_sh_eta[i][j] = ibooker.book1D( rh_sh_eta_name.c_str(), rh_sh_eta_title.c_str(), 140, -minEta_, maxEta_) ;
      me_rh_sh_eta_[i][j] = ibooker.book1D( rh_sh_eta_name.c_str(), rh_sh_eta_title.c_str(), 101, -3, 3) ;

      for ( unsigned int k = 0 ; k < c_suffix.size() ; k++) {
        suffix = string(s_suffix[j])+string(l_suffix[i])+ string(c_suffix[k]);
        string rh_phi_name = string("rh_phi")+suffix;
        string rh_phi_title = rh_phi_name+"; tracks #phi; # of tracks";
        me_rh_phi_[i][j][k] = ibooker.book1D( (rh_phi_name).c_str(), rh_phi_title.c_str(), 200, -PI,PI) ;

        string rh_sh_phi_name = string("rh_sh_phi")+suffix;
        string rh_sh_phi_title = rh_sh_phi_name+"; tracks #phi; # of tracks";
        me_rh_sh_phi_[i][j][k] = ibooker.book1D( (rh_sh_phi_name).c_str(), rh_sh_phi_title.c_str(), 200,-PI,PI) ;

      }
    }
  }


  me_region_ = ibooker.book1D("region", "region", 5, -2 - 0.5, 2 + 0.5);
  me_station_ = ibooker.book1D("station", "station",  5, -2 - 0.5, 2 + 0.5);
  me_ring_ = ibooker.book1D("ring", "ring",  5, -2 - 0.5, 2 + 0.5);
  me_layer_ = ibooker.book1D("layer", "layer",  4, 0 - 0.5, 3 + 0.5);
  me_chamber_ = ibooker.book1D("chamber", "chamber",  51, 0 - 0.5, 50 + 0.5);
  me_roll_ = ibooker.book1D("roll", "roll",  10, 0 - 0.5, 9 + 0.5);

  me_eta_phi_ = ibooker.book2D("eta_phi", "eta_phi", 101, -4, 4, 101, -3.14, 3.14);
  me_eta_phi_sim_hit_ = ibooker.book2D("eta_phi_sim_hit", "eta_phi_sim_hit", 101, -4, 4, 101, -3.14, 3.14);
  me_eta_phi_rec_hit_ = ibooker.book2D("eta_phi_rec_hit", "eta_phi_rec_hit", 101, -4, 4, 101, -3.14, 3.14);
  me_eta_phi_track_ = ibooker.book2D("eta_phi_track", "eta_phi_track", 101, -4, 4, 101, -3.14, 3.14);
  me_eta_phi_good_track_ = ibooker.book2D("eta_phi_good_track", "eta_phi_good_track", 101, -4, 4, 101, -3.14, 3.14);


  me_num_matched_sim_hits_ = ibooker.book1D("num_matched_sim_hits", "num_matched_sim_hits", 11, 0 - 0.5, 10 + 0.5);
  me_num_matched_sim_tracks_ = ibooker.book1D("num_matched_sim_tracks", "num_matched_sim_tracks", 11, 0 - 0.5, 10 + 0.5);


  // const DetIdContainer &
  for(auto & det_id : kGEMGeom.detUnitIds())
  {
    GEMDetId gem_id(det_id);

    me_region_->Fill(gem_id.region());
    me_station_->Fill(gem_id.station());
    me_ring_->Fill(gem_id.ring());
    me_layer_->Fill(gem_id.layer());
    me_chamber_->Fill(gem_id.chamber());
    me_roll_->Fill(gem_id.roll());

    const GEMEtaPartition* eta_partition = kGEMGeom.etaPartition(gem_id);
    LocalPoint local_origin(0.0, 0.0, 0.0);
    GlobalPoint gp = eta_partition->toGlobal(local_origin);
    auto eta = gp.eta();
    auto phi = gp.phi();
    me_eta_phi_->Fill(eta, phi);
  }
}

GEMRecHitTrackMatch::~GEMRecHitTrackMatch() {  }


void GEMRecHitTrackMatch::analyze(const edm::Event& iEvent,
                                  const edm::EventSetup& iSetup)
{
  edm::LogInfo("GEMRecHitTrackMatch")<<"GEM RecHitTrackMatch :: analyze"<<std::endl;
  edm::ESHandle<GEMGeometry> hGeom;
  iSetup.get<MuonGeometryRecord>().get(hGeom);
  const GEMGeometry& kGEMGeom = *hGeom;

  edm::Handle<edm::PSimHitContainer> sim_hits;
  edm::Handle<edm::SimTrackContainer> sim_tracks;
  edm::Handle<GEMRecHitCollection> rec_hits;
  //edm::Handle<edm::SimVertexContainer> sim_vertices;
  iEvent.getByToken(simHitsToken_, sim_hits);
  iEvent.getByToken(simTracksToken_, sim_tracks);
  iEvent.getByToken(gem_recHitToken_, rec_hits);

  // TODO error log
  if ( not sim_hits.isValid())
  {
    edm::LogError("GEMRecHitTrackMatch") << "Invalid SimHit" << std::endl;
    return;
  }

  if ( not sim_tracks.isValid() )
  {
    edm::LogError("GEMRecHitTrackMatch") << "Invalid SimTrack" << std::endl;
    return;
  }

  if(not rec_hits.isValid())
  {
    edm::LogError("GEMRecHitTrackMatch") << "Invalid RecHit" << std::endl;
    return;
  }


  //const edm::SimVertexContainer & sim_vert = *sim_vertices.product();
  // const edm::SimTrackContainer & sim_track_container = *sim_tracks.product();
  // const edm::PSimHitContainer & sim_hit_container = *sim_hits.product();

  // Consider only SimTracks that matche 
  // edm::SimTrackContainer matched_sim_track_container;  
  std::vector<std::pair<SimTrack, std::vector<PSimHit> > > matched_track_hits;
  // for(edm::SimTrackContainer::const_iterator sim_track=sim_tracks->begin(); sim_track != sim_tracks->end(); ++sim_track)
  for(auto sim_track = sim_tracks->begin(); sim_track != sim_tracks->end(); ++sim_track)
  {
    if(not isSimTrackGood(*sim_track)) continue;

    std::vector<PSimHit> matched_hits;
    for (edm::PSimHitContainer::const_iterator sim_hit = sim_hits->begin(); sim_hit != sim_hits->end(); ++sim_hit)
    {
      
      if(sim_track->trackId() == sim_hit->trackId())
      {
        // Local3DPoint sh_lp = sim_hit->localPosition();
        DetId det_id(sim_hit->detUnitId());

        // constructor of GEMDetId check if detUnitId of sim_hit is valid
        // if not, it throw cms::Exception("InvalidDetId")
        GEMDetId sim_det_id(det_id);
        std::cout << "region: " << sim_det_id.region()
                  << " | ring: " << sim_det_id.ring()
                  << " | station: " << sim_det_id.station()
                  << " | layer: " << sim_det_id.layer()
                  << " | chamber: " << sim_det_id.chamber()
                  << " | layer: " << sim_det_id.layer()
                  << std::endl;
        // matched_sim_track_container.push_back(*sim_track);

        matched_hits.push_back(*sim_hit);
      } 
    }

    me_num_matched_sim_hits_->Fill(matched_hits.size());

    if(matched_hits.empty())
        continue;
    else
        matched_track_hits.push_back(std::make_pair(*sim_track, matched_hits));
  }

  me_num_matched_sim_tracks_->Fill(matched_track_hits.size());


  if(matched_track_hits.empty())
  {
    edm::LogError("GEMRecHitTrackMatch") << "No SimTrack matched SimHits" << std::endl;
    return;
  }


  // XXX
  GEMSimTrackHelper trk_helper;
  for(auto& each : matched_track_hits)
  {
    std::set<unsigned int> sim_hit_det_ids;
    std::set<unsigned int> rec_hit_det_ids;

    for(PSimHit& sim_hit : each.second)
    {
      unsigned int det_id = sim_hit.detUnitId();
      GEMDetId gem_id(det_id);
      sim_hit_det_ids.insert(det_id);

      LocalPoint lp = sim_hit.localPosition();
      Int_t sim_fired_strip = kGEMGeom.etaPartition(gem_id)->strip(lp) + 1;

      // 
      GEMRecHitCollection::range range = rec_hits->get(gem_id);
      for(GEMRecHitCollection::const_iterator rec_hit = range.first; rec_hit != range.second; ++rec_hit)
      {
        Int_t rec_cls = rec_hit->clusterSize();

        // Checkt whether a sim. fired strip is in a rec. cluster strips.
        bool sh_rh_matched;
        if ( rec_cls == 1 )
        {
          sh_rh_matched = sim_fired_strip == rec_hit->firstClusterStrip();
        }
        else
        {
          Int_t rec_first_fired_strip = rec_hit->firstClusterStrip();
          Int_t rec_last_fired_strip = rec_first_fired_strip + rec_cls - 1;
          sh_rh_matched = (sim_fired_strip >= rec_first_fired_strip) and (sim_fired_strip <= rec_last_fired_strip);
        }

        if(sh_rh_matched)
        {
          rec_hit_det_ids.insert(det_id);
          break;
        }
      } // END RecHit Loop
    } // END SimHit Loop

    const PSimHit & kFirstSimHit = each.second[0];
    DetId det_id(kFirstSimHit.detUnitId());
    LocalPoint lp = kFirstSimHit.localPosition();
    GlobalPoint gp = kGEMGeom.idToDet(det_id)->surface().toGlobal(lp);

    const float kEta = gp.eta();
    const float kPhi = gp.phi();
    me_eta_phi_track_->Fill(kEta, kPhi);

    trk_helper.reset();
    trk_helper.setValuesWithSimHitChamberIds(sim_hit_det_ids);
    trk_helper.setValuesWithRecHitChamberIds(rec_hit_det_ids);

    //track
    FillWithTrigger(me_track_eta_, kEta);
    FillWithTrigger(me_track_phi_, kEta, kPhi, trk_helper.hitOdd, trk_helper.hitEven);

    FillWithTrigger(me_rh_sh_eta_, trk_helper.gem_sh, kEta);
    FillWithTrigger(me_rh_eta_,    trk_helper.gem_rh, kEta);
  
    // Separate station.
    FillWithTrigger(me_rh_sh_phi_, trk_helper.gem_sh, kEta, kPhi, trk_helper.hitOdd, trk_helper.hitEven);
    FillWithTrigger(me_rh_phi_,    trk_helper.gem_rh, kEta, kPhi, trk_helper.hitOdd, trk_helper.hitEven);
  } // END SimTrack Loop
}
