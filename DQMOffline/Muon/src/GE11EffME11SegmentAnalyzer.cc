#include "DQMOffline/Muon/interface/GE11EffME11SegmentAnalyzer.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/ErrorFrameTransformer.h"
#include "DataFormats/GeometrySurface/interface/SimpleDiskBounds.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartitionSpecs.h"
#include "Validation/MuonHits/interface/MuonHitHelper.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"


GE11EffME11SegmentAnalyzer::GE11EffME11SegmentAnalyzer(const edm::ParameterSet& parameter_set)
    : GEMOfflineDQMBase(parameter_set),
      kGEMToken_(esConsumes<GEMGeometry, MuonGeometryRecord>()),
      kGEMTokenBeginRun_(esConsumes<edm::Transition::BeginRun>()),
      kCSCToken_(esConsumes<CSCGeometry, MuonGeometryRecord>()),
      kGlobalTrackingGeometryToken_(esConsumes<GlobalTrackingGeometry, GlobalTrackingGeometryRecord>()),
      kTrasientTrackToken_(esConsumes<TransientTrackBuilder, TransientTrackRecord>(edm::ESInputTag("", "TransientTrackBuilder"))),
      kRecHitToken_(consumes<GEMRecHitCollection>(parameter_set.getParameter<edm::InputTag>("recHitTag"))),
      kMuonToken_(consumes<edm::View<reco::Muon> >(parameter_set.getParameter<edm::InputTag>("muonTag"))),
      kMuonTrackType_(getMuonTrackType(parameter_set.getUntrackedParameter<std::string>("muonTrackType"))),
      kIsCosmics_(parameter_set.getUntrackedParameter<bool>("isCosmics")),
      kMinRdPhi_(static_cast<float>(parameter_set.getUntrackedParameter<double>("minRdPhi"))),
      kFiducialCutY_(static_cast<float>(parameter_set.getUntrackedParameter<double>("fiducialCutY"))),
      kFiducialCutPhi_(static_cast<float>(parameter_set.getUntrackedParameter<double>("fiducialCutPhi"))),
      kFolder_(parameter_set.getUntrackedParameter<std::string>("folder"))
{

  muon_service_ = std::make_unique<MuonServiceProxy>(
      parameter_set.getParameter<edm::ParameterSet>("ServiceParameters"),
      consumesCollector());
}



GE11EffME11SegmentAnalyzer::~GE11EffME11SegmentAnalyzer() {}

void GE11EffME11SegmentAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("recHitTag", edm::InputTag("gemRecHits"));
  desc.add<edm::InputTag>("muonTag", edm::InputTag("muons"));
  desc.addUntracked<std::string>("muonTrackType", "InnerTrack");
  desc.addUntracked<bool>("isCosmics", false);
  desc.addUntracked<double>("fiducialCutY", 5.0);
  desc.addUntracked<double>("fiducialCutPhi", 1.0);
  desc.addUntracked<double>("minRdPhi", 2.0);
  desc.addUntracked<std::string>("folder", "GEM/Efficiency/ME11Segment");
  {
    edm::ParameterSetDescription psd0;
    psd0.setAllowAnything();
    desc.add<edm::ParameterSetDescription>("ServiceParameters", psd0);
  }
  descriptions.add("GE11EffME11SegmentAnalyzerDefault", desc);
}

void GE11EffME11SegmentAnalyzer::bookHistograms(DQMStore::IBooker& booker,
                                               edm::Run const& run,
                                           edm::EventSetup const& setup) {
  const edm::ESHandle<GEMGeometry>&& gem = setup.getHandle(kGEMTokenBeginRun_);
  if (not gem.isValid()) {
    edm::LogError(kLogCategory_) << "GEMGeometry is invalid";
    return;
  }

  booker.setCurrentFolder(kFolder_ + "/Efficiency");
  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();

    const std::vector<const GEMSuperChamber*>&& superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs";
      return;
    }

    // loop over layers
    const int num_chambers = superchambers.size();
    for (const GEMChamber* chamber : superchambers.at(0)->chambers()) {
      const int layer_id = chamber->id().layer();

      const TString name = "chamber" + GEMUtils::getSuffixName(region_id, station_id, layer_id);
      const TString title = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
      const GEMDetId&& key = getReStLaKey(chamber->id());

      me_chamber_[key] = booker.book1D(name, title, num_chambers, 0.5, num_chambers + 0.5);
      me_chamber_[key]->setAxisTitle("Chamber");
      me_chamber_[key]->getTH1F()->SetNdivisions(-num_chambers, "Y");
      for (int binx = 1; binx <= num_chambers; binx++) {
        me_chamber_[key]->setBinLabel(binx, std::to_string(binx));
      }

      me_chamber_matched_[key] = bookNumerator1D(booker, me_chamber_[key]);
    }  // layer
  }    // station

}


void GE11EffME11SegmentAnalyzer::analyze(const edm::Event& event, const edm::EventSetup& setup) {
  const edm::Handle<GEMRecHitCollection>&& rechit_collection = event.getHandle(kRecHitToken_);
  if (not rechit_collection.isValid()) {
    edm::LogError(kLogCategory_) << "GEMRecHitCollection is invalid";
    return;
  }

  const edm::Handle<edm::View<reco::Muon> >&& muon_collection = event.getHandle(kMuonToken_);
  if (not muon_collection.isValid()) {
    edm::LogError(kLogCategory_) << "View<Muon> is invalid";
    return;
  }

  const edm::ESHandle<GEMGeometry>&& gem = setup.getHandle(kGEMToken_);
  if (not gem.isValid()) {
    edm::LogError(kLogCategory_) << "GEMGeometry is invalid";
    return;
  }

  const edm::ESHandle<CSCGeometry>&& csc = setup.getHandle(kCSCToken_);
  if (not csc.isValid()) {
    edm::LogError(kLogCategory_) << "CSCGeometry is invalid";
    return;
  }

  const edm::ESHandle<GlobalTrackingGeometry>&& global_tracking_geometry = setup.getHandle(kGlobalTrackingGeometryToken_);
  if (not global_tracking_geometry.isValid()) {
    edm::LogError(kLogCategory_) << "GlobalTrackingGeometry is invalid";
    return;
  }

  const edm::ESHandle<TransientTrackBuilder>&& transient_track_builder = setup.getHandle(kTrasientTrackToken_);
  if (not transient_track_builder.isValid()) {
    edm::LogError(kLogCategory_) << "TransientTrackRecord is invalid";
    return;
  }

  muon_service_->update(setup);
  const edm::ESHandle<Propagator>&& propagator = muon_service_->propagator("SteppingHelixPropagatorAny");
  if (not propagator.isValid()) {
    edm::LogError(kLogCategory_) << "Propagator is invalid";
    return;
  }

  //////////////////////////////////////////////////////////////////////////////
  if (muon_collection->empty()) {
    edm::LogInfo(kLogCategory_) << "empty muon collection";
    return;
  }

  //////////////////////////////////////////////////////////////////////////////
  for (const reco::Muon& muon : *muon_collection) {
    if (not muon.isStandAloneMuon()) {
      edm::LogError(kLogCategory_) << "failed to pass reco::Muon::isStandAloneMuon";
      continue;
    }

    propagate(muon, transient_track_builder, propagator, rechit_collection, gem, csc);
  }

}

void GE11EffME11SegmentAnalyzer::propagate(
    const reco::Muon& muon,
    const edm::ESHandle<TransientTrackBuilder>& transient_track_builder,
    const edm::ESHandle<Propagator>& propagator,
    const edm::Handle<GEMRecHitCollection>& rechit_collection,
    const edm::ESHandle<GEMGeometry>& gem,
    const edm::ESHandle<CSCGeometry>& csc) {

  const reco::Track* track = muon.muonTrack(kMuonTrackType_).get();
  if (track == nullptr) {
    edm::LogInfo(kLogCategory_) << "failed to get a track from muon";
    return;
  }

  const reco::TransientTrack&& transient_track = transient_track_builder->build(track);
  if (not transient_track.isValid()) {
    edm::LogError(kLogCategory_) << "failed to build TransientTrack";
    return;
  }

  const CSCSegment* me11_segment = findME11Segment(muon);
  if (me11_segment == nullptr) {
    edm::LogInfo(kLogCategory_) << "failed to find a ME11 segment";
    return;
  }

  const TrajectoryStateOnSurface&& starting_state = getStartingState(me11_segment, muon, csc);
  if (not starting_state.isValid()) {
    edm::LogError(kLogCategory_) << "got an invalid starting state";
  }

  for (const GEMEtaPartition* eta_partition : gem->etaPartitions()) {
    if (not eta_partition->isGE11()) {
      continue;
    }

    const GEMDetId&& gem_id = eta_partition->id();
    const GEMDetId&& rsl_key = getReStLaKey(gem_id);
    const int chamber_id = gem_id.chamber();

    const TrajectoryStateOnSurface&& propagated_state = propagator->propagate(
        starting_state, eta_partition->surface());
    if (not isGoodPropagation(starting_state, eta_partition, propagated_state)) {
      edm::LogInfo(kLogCategory_) << "bad propagation";
      continue;
    }

    fillME(me_chamber_, rsl_key, chamber_id);

    const auto& [matched_hit, rdphi] = findClosestHit(starting_state.globalPosition(),
                                                      eta_partition,
                                                      rechit_collection,
                                                      gem);
    if ((matched_hit == nullptr) or (rdphi > kMinRdPhi_)) {
      continue;
    }

    fillME(me_chamber_matched_, rsl_key, chamber_id);
  }
}

const CSCSegment* GE11EffME11SegmentAnalyzer::findME11Segment(const reco::Muon& muon) {
  const CSCSegment* me11_segment = nullptr;

  if (kIsCosmics_) {
    for (const reco::MuonChamberMatch& chamber_match : muon.matches()) {
      if (chamber_match.detector() != MuonSubdetId::CSC) {
        continue;
      }

      for (const reco::MuonSegmentMatch& segment_match : chamber_match.segmentMatches) {
        const CSCDetId&& csc_id = segment_match.cscSegmentRef->cscDetId();
        if (isME11(csc_id)) {
          me11_segment = segment_match.cscSegmentRef.get();
        }
      }
    }

  } else {
    // pp or HeavyIons
    const reco::Track* track = muon.outerTrack().get();
    for (const TrackingRecHit* tracking_rechit : track->recHits()) {
      if (isME11(tracking_rechit->geographicalId())) {
        if(tracking_rechit->dimension() == 4) {
          // https://github.com/cms-sw/cmssw/blob/master/DataFormats/CSCRecHit/interface/CSCSegment.h#L60
          me11_segment = dynamic_cast<const CSCSegment*>(tracking_rechit);
        }
      }
    }

  }

  return me11_segment;
}


TrajectoryStateOnSurface GE11EffME11SegmentAnalyzer::getStartingState(
    const CSCSegment* me11_segment,
    const reco::Muon& muon,
    const edm::ESHandle<CSCGeometry>& csc) {

  // TODO why innertrack?
  const reco::TrackRef track = muon.track();
  // no momentum for segments
  // If innerTrack exists, use momentum
  const float momentum_magnitude = track.isNull() ? 1.0f : track->outerP();
  const LocalVector momentum = momentum_magnitude * me11_segment->localDirection();
  const LocalTrajectoryParameters param(me11_segment->localPosition(), momentum, muon.charge());

  const AlgebraicSymMatrix error_matrix = me11_segment->parametersError().similarityT(me11_segment->projectionMatrix());
  const LocalTrajectoryError error(asSMatrix<5>(error_matrix));

  const Plane& surface = csc->idToDet(me11_segment->geographicalId())->surface();

  const edm::ESHandle<MagneticField> magnetic_field = muon_service_->magneticField();

  TrajectoryStateOnSurface starting_state(param, error, surface, magnetic_field.product());

  return starting_state;
}


bool GE11EffME11SegmentAnalyzer::isGoodPropagation(
    const TrajectoryStateOnSurface& starting_state,
    const GEMEtaPartition* destination,
    const TrajectoryStateOnSurface& propagated_state) {
  if (not propagated_state.isValid()) {
    edm::LogError(kLogCategory_) << "The propagated segment is invalid";
    return false;
  } 

  if (propagated_state.globalPosition().z() * starting_state.globalPosition().z() < 0) {
    edm::LogInfo(kLogCategory_) << "ME11 segment is propagated to the opposite region";
    return false;
  }

  if (not checkBounds(propagated_state.globalPosition(), destination->surface())) {
    edm::LogInfo(kLogCategory_) << "The propagated ME11 segment is outside the destination";
    return false;
  }

  if (not checkFiducialCut(propagated_state.globalPosition(), destination)) {
    edm::LogInfo(kLogCategory_) << "failed to pass fiducial cuts";
  }

  return true;
}

bool GE11EffME11SegmentAnalyzer::checkBounds(const GlobalPoint& global_point, const Plane& plane) {
  const LocalPoint&& local_point = plane.toLocal(global_point);
  const LocalPoint local_point_2d(local_point.x(), local_point.y(), 0.0f);
  return plane.bounds().inside(local_point_2d);
}

bool GE11EffME11SegmentAnalyzer::checkFiducialCut(const GlobalPoint& global_position,
                                                  const GEMEtaPartition* eta_partition) {
  const int ieta = eta_partition->id().ieta();

  // https://github.com/cms-sw/cmssw/blob/CMSSW_12_2_0_pre1/Geometry/GEMGeometry/src/GEMEtaPartitionSpecs.cc#L12
  // The height parameter is half the strip length.
  const float height = eta_partition->specs()->parameters().at(2);
  const float y_cut = height - kFiducialCutY_;
  const float angle_cut = 5.0 - kFiducialCutPhi_; // FIXME 5.1?

  const LocalPoint&& local_position = eta_partition->toLocal(global_position);
  const float local_y = local_position.y();
  const float local_phi = TMath::RadToDeg() * eta_partition->specificTopology().stripAngle(eta_partition->strip(local_position));

  if (std::abs(local_phi_deg) > angle_cut) {
    return false;
  }

  if ((ieta == 1) and (local_y > y_cut)) {
    return false;

  } else if ((ieta == GEMeMap::maxiEtaIdGE11_) and (local_y < -1.0 * y_cut)) {
    return false;

  }

  // pass fiducial cuts
  return true;
}


// Returns
std::pair<const GEMRecHit*, float> GE11EffME11SegmentAnalyzer::findClosestHit(
    const GlobalPoint& prop_global_pos,
    const GEMEtaPartition* prop_eta_partition,
    const edm::Handle<GEMRecHitCollection>& rechit_collection,
    const edm::ESHandle<GEMGeometry>& gem) {
  const GEMDetId&& prop_gem_id = prop_eta_partition->id();
  const int prop_ieta = prop_gem_id.ieta();

  const GEMDetId&& chamber_id = prop_gem_id.chamberId();
  const GEMChamber* chamber = gem->chamber(chamber_id);
  if (chamber == nullptr) {
    edm::LogError(kLogCategory_) << chamber_id;
    return std::make_pair(nullptr, 1e6f);
  }
  const LocalPoint&& prop_chamber_pos = chamber->toLocal(prop_global_pos);

  const GEMRecHit* closest_hit = nullptr;
  float min_rdphi = 1e6f; // just big number

  for (const GEMEtaPartition* hit_eta_partition : chamber->etaPartitions()) {
    const GEMDetId&& hit_gem_id = hit_eta_partition->id();
    const int delta_ieta = std::abs(prop_ieta - hit_gem_id.ieta());
    if (delta_ieta > 1) {
      continue;
    }

    const GEMRecHitCollection::range& rechit_range = rechit_collection->get(hit_gem_id);
    for (auto hit = rechit_range.first; hit != rechit_range.second; ++hit) {
      const GlobalPoint&& hit_global_pos = hit_eta_partition->toGlobal(hit->localPosition());
      const LocalPoint&& hit_chamber_pos = chamber->toLocal(hit_global_pos);

      const float hit_strip = hit_eta_partition->strip(hit->localPosition());
      const float hit_strip_angle = hit_eta_partition->specificTopology().stripAngle(hit_strip);

      const float dx = prop_chamber_pos.x() - hit_chamber_pos.x();
      const float dy = prop_chamber_pos.y() - hit_chamber_pos.y();

      const float rdphi = std::cos(hit_strip_angle) * dx - std::sin(hit_strip_angle) * dy;

      if (rdphi < min_rdphi) {
        closest_hit = &(*hit);
        min_rdphi = rdphi;
      }
    }
  }

  return std::make_pair(closest_hit, min_rdphi);
}


dqm::impl::MonitorElement* GE11EffME11SegmentAnalyzer::bookNumerator1D(DQMStore::IBooker& booker, MonitorElement* me) {
  const std::string name = me->getName() + "_matched";
  TH1F* hist = dynamic_cast<TH1F*>(me->getTH1F()->Clone(name.c_str()));
  return booker.book1D(name, hist);
}

dqm::impl::MonitorElement* GE11EffME11SegmentAnalyzer::bookNumerator2D(DQMStore::IBooker& booker, MonitorElement* me) {
  const std::string name = me->getName() + "_matched";
  TH2F* hist = dynamic_cast<TH2F*>(me->getTH2F()->Clone(name.c_str()));
  return booker.book2D(name, hist);
}

reco::Muon::MuonTrackType GE11EffME11SegmentAnalyzer::getMuonTrackType(const std::string& name) {
  reco::Muon::MuonTrackType muon_track_type;

  // NOTE do NOT allow typo
  if (name == "InnerTrack") {
    // tracker
    muon_track_type = reco::Muon::MuonTrackType::InnerTrack;

  } else if (name == "OuterTrack") {
    // standalone
    muon_track_type = reco::Muon::MuonTrackType::OuterTrack;

  } else if (name == "CombinedTrack") {
    // global
    muon_track_type = reco::Muon::MuonTrackType::CombinedTrack;

  } else {
    muon_track_type = reco::Muon::MuonTrackType::None;
    edm::LogError(kLogCategory_) << "got a wrong MuonTrackType label: "
                                 << name.c_str()
                                 << ". All events will be skipped.";
  }

  return muon_track_type;
}

bool GE11EffME11SegmentAnalyzer::isME11(const CSCDetId& csc_id) {
  // https://github.com/cms-sw/cmssw/blob/CMSSW_12_2_0_pre1/DataFormats/MuonDetId/interface/CSCDetId.h#L278
  return csc_id.station() == 1 and (csc_id.ring() == 1 or csc_id.ring() == 4);
}

bool GE11EffME11SegmentAnalyzer::isME11(const DetId& det_id) {
  if (MuonHitHelper::isCSC(det_id)) {
    const CSCDetId csc_id{det_id};
    return isME11(csc_id);
  } else {
    return false;
  }
}
