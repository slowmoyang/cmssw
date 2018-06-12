#ifndef GEMRecHitTrackMatch_H
#define GEMRecHitTrackMatch_H

#include "Validation/MuonGEMHits/interface/GEMTrackMatch.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartition.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartitionSpecs.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"
#include "Validation/MuonGEMHits/interface/SimHitMatcher.h"
#include "Validation/MuonGEMDigis/interface/GEMDigiMatcher.h"
#include "Validation/MuonGEMRecHits/interface/GEMRecHitMatcher.h"

#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"


class GEMRecHitTrackMatch : public GEMTrackMatch 
{
public:
  explicit GEMRecHitTrackMatch(const edm::ParameterSet& ps);
  ~GEMRecHitTrackMatch() override;
  void bookHistograms(DQMStore::IBooker&, edm::Run const&, edm::EventSetup const &) override;
  void analyze(const edm::Event& e, const edm::EventSetup&) override;
 private:

  MonitorElement* me_track_eta_[3];
  MonitorElement* me_track_phi_[3][3];
  MonitorElement* me_rh_eta_[4][3];
  MonitorElement* me_rh_sh_eta_[4][3]; 
  MonitorElement* me_rh_phi_[4][3][3];
  MonitorElement* me_rh_sh_phi_[4][3][3]; 

  MonitorElement *me_region_;
  MonitorElement *me_station_;
  MonitorElement *me_ring_;
  MonitorElement *me_layer_;
  MonitorElement *me_chamber_;
  MonitorElement *me_roll_;

  MonitorElement *me_eta_phi_;
  MonitorElement *me_eta_phi_sim_hit_;
  MonitorElement *me_eta_phi_rec_hit_;
  MonitorElement *me_eta_phi_track_;
  MonitorElement *me_eta_phi_good_track_;
  MonitorElement *me_num_matched_sim_hits_;
  MonitorElement *me_num_matched_sim_tracks_;

  edm::EDGetToken gem_recHitToken_;
};

#endif
