#include "DQMOffline/Muon/interface/GEMEfficiencyAnalyzer.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/ErrorFrameTransformer.h"
#include "DataFormats/GeometrySurface/interface/SimpleDiskBounds.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"
#include "Validation/MuonHits/interface/MuonHitHelper.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"

GEMEfficiencyAnalyzer::GEMEfficiencyAnalyzer(const edm::ParameterSet& pset)
    : GEMOfflineDQMBase(pset),
      kGEMTokenBeginRun_(esConsumes<edm::Transition::BeginRun>()),
      kGEMToken_(esConsumes<GEMGeometry, MuonGeometryRecord>()),
      kGlobalTrackingGeometryToken_(esConsumes<GlobalTrackingGeometry, GlobalTrackingGeometryRecord>()),
      kTransientTrackBuilderToken_(esConsumes<TransientTrackBuilder, TransientTrackRecord>(edm::ESInputTag("", "TransientTrackBuilder"))),
      kGEMRecHitToken_(consumes<GEMRecHitCollection>(pset.getParameter<edm::InputTag>("recHitTag"))),
      kMuonToken_(consumes<edm::View<reco::Muon> >(pset.getParameter<edm::InputTag>("muonTag"))),
      kMuonTrackType_(getMuonTrackType(pset.getUntrackedParameter<std::string>("muonTrackType"))),
      kMuonName_(pset.getUntrackedParameter<std::string>("muonName")),
      kFolder_(pset.getUntrackedParameter<std::string>("folder")),
      kIsCosmics_(pset.getUntrackedParameter<bool>("isCosmics")),
      kUseSkipLayer_(pset.getParameter<bool>("useSkipLayer")),
      kUseOnlyME11_(pset.getParameter<bool>("useOnlyME11")),
      kResidualRPhiCut_(static_cast<float>(pset.getParameter<double>("residualRPhiCut"))),
      kUsePropRErrorCut_(pset.getParameter<bool>("usePropRErrorCut")),
      kPropRErrorCut_(pset.getParameter<double>("propRErrorCut")),
      kUsePropPhiErrorCut_(pset.getParameter<bool>("usePropPhiErrorCut")),
      kPropPhiErrorCut_(pset.getParameter<double>("propPhiErrorCut")),
      kPtBins_(pset.getUntrackedParameter<std::vector<double> >("ptBins")),
      kEtaNBins_(pset.getUntrackedParameter<int>("etaNbins")),
      kEtaLow_(pset.getUntrackedParameter<double>("etaLow")),
      kEtaUp_(pset.getUntrackedParameter<double>("etaUp")) {

  const edm::ParameterSet&& muon_service_parameter = pset.getParameter<edm::ParameterSet>("ServiceParameters");
  muon_service_ = new MuonServiceProxy(muon_service_parameter, consumesCollector());

}

GEMEfficiencyAnalyzer::~GEMEfficiencyAnalyzer() {}

void GEMEfficiencyAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  // beam scenario
  {
    edm::ParameterSetDescription desc;
    desc.addUntracked<std::string>("muonTrackType", "CombinedTrack");
    desc.addUntracked<std::string>("muonName", "Global Muon");
    desc.addUntracked<std::string>("folder", "GEM/Efficiency/type0");
    desc.add<edm::InputTag>("recHitTag", edm::InputTag("gemRecHits"));
    desc.add<edm::InputTag>("muonTag", edm::InputTag("muons"));
    desc.addUntracked<bool>("isCosmics", false);
    desc.add<bool>("useSkipLayer", true);
    desc.add<bool>("useOnlyME11", false);
    desc.add<double>("residualRPhiCut", 2.0);  // TODO need to be tuned
    desc.add<bool>("usePropRErrorCut", false);
    desc.add<double>("propRErrorCut", 1.0);
    desc.add<bool>("usePropPhiErrorCut", false);
    desc.add<double>("propPhiErrorCut", 0.01);
    desc.addUntracked<std::vector<double> >("ptBins", {20., 30., 40., 50., 60., 70., 80., 90., 100., 120.});
    desc.addUntracked<int>("etaNbins", 9);
    desc.addUntracked<double>("etaLow", 1.4);
    desc.addUntracked<double>("etaUp", 2.3);
    {
      edm::ParameterSetDescription psd0;
      psd0.setAllowAnything();
      desc.add<edm::ParameterSetDescription>("ServiceParameters", psd0);
    }
    descriptions.add("gemEfficiencyAnalyzerDefault", desc);
  }  // beam scenario

  // cosmic scenario
  {
    edm::ParameterSetDescription desc;
    desc.addUntracked<std::string>("muonTrackType", "CombinedTrack");
    desc.addUntracked<std::string>("muonName", "Global Muon");
    desc.addUntracked<std::string>("folder", "GEM/Efficiency/type0");
    desc.add<edm::InputTag>("recHitTag", edm::InputTag("gemRecHits"));
    desc.add<edm::InputTag>("muonTag", edm::InputTag("muons"));
    desc.addUntracked<bool>("isCosmics", true);
    desc.add<bool>("useSkipLayer", true);
    desc.add<bool>("useOnlyME11", true);
    desc.add<double>("residualRPhiCut", 5.0);  // TODO need to be tuned
    desc.add<bool>("usePropRErrorCut", true);
    desc.add<double>("propRErrorCut", 1.0);
    desc.add<bool>("usePropPhiErrorCut", true);
    desc.add<double>("propPhiErrorCut", 0.001);
    desc.addUntracked<std::vector<double> >("ptBins", {0., 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., 120.});
    desc.addUntracked<int>("etaNbins", 9);
    desc.addUntracked<double>("etaLow", 1.4);
    desc.addUntracked<double>("etaUp", 2.3);
    {
      edm::ParameterSetDescription psd0;
      psd0.setAllowAnything();
      desc.add<edm::ParameterSetDescription>("ServiceParameters", psd0);
    }
    descriptions.add("gemEfficiencyAnalyzerCosmicsDefault", desc);
  }  // cosmics
}

void GEMEfficiencyAnalyzer::bookHistograms(DQMStore::IBooker& ibooker,
                                           edm::Run const& run,
                                           edm::EventSetup const& isetup) {
  edm::ESHandle<GEMGeometry> gem = isetup.getHandle(kGEMTokenBeginRun_);
  if (not gem.isValid()) {
    edm::LogError(kLogCategory_) << "GEMGeometry is invalid" << std::endl;
    return;
  }

  bookEfficiencyMomentum(ibooker, gem);
  bookEfficiencyChamber(ibooker, gem);
  bookEfficiencyEtaPartition(ibooker, gem);
  bookEfficiencyDetector(ibooker, gem);
  bookResolution(ibooker, gem);
  bookMisc(ibooker, gem);
}

dqm::impl::MonitorElement* GEMEfficiencyAnalyzer::bookNumerator1D(DQMStore::IBooker& ibooker, MonitorElement* me) {
  const std::string name = me->getName() + "_matched";
  TH1F* hist = dynamic_cast<TH1F*>(me->getTH1F()->Clone(name.c_str()));
  return ibooker.book1D(name, hist);
}

dqm::impl::MonitorElement* GEMEfficiencyAnalyzer::bookNumerator2D(DQMStore::IBooker& ibooker, MonitorElement* me) {
  const std::string name = me->getName() + "_matched";
  TH2F* hist = dynamic_cast<TH2F*>(me->getTH2F()->Clone(name.c_str()));
  return ibooker.book2D(name, hist);
}

void GEMEfficiencyAnalyzer::bookEfficiencyMomentum(DQMStore::IBooker& ibooker, const edm::ESHandle<GEMGeometry>& gem) {
  // TODO Efficiency/Source
  ibooker.setCurrentFolder(kFolder_ + "/Efficiency");

  const TString pt_x_title = "Muon p_{T} [GeV]";
  const int pt_nbinsx = kPtBins_.size() - 1;

  const std::string eta_x_title = "Muon |#eta|";
  const std::string phi_x_title = "Muon #phi [rad]";

  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();

    const GEMDetId&& key = getReStKey(region_id, station_id);
    const TString&& name_suffix = GEMUtils::getSuffixName(region_id, station_id);
    const TString&& title_suffix = GEMUtils::getSuffixTitle(region_id, station_id);

    const TString&& title = kMuonName_.c_str() + title_suffix;

    TH1F* h_muon_pt = new TH1F("muon_pt" + name_suffix, title, pt_nbinsx, &kPtBins_.at(0));
    h_muon_pt->SetXTitle(pt_x_title);
    me_muon_pt_[key] = ibooker.book1D(h_muon_pt->GetName(), h_muon_pt);
    me_muon_pt_matched_[key] = bookNumerator1D(ibooker, me_muon_pt_.at(key));

    me_muon_eta_[key] = ibooker.book1D("muon_eta" + name_suffix, title, kEtaNBins_, kEtaLow_, kEtaUp_);
    me_muon_eta_.at(key)->setXTitle(eta_x_title);
    me_muon_eta_matched_[key] = bookNumerator1D(ibooker, me_muon_eta_.at(key));

    me_muon_phi_[key] = ibooker.book1D("muon_phi" + name_suffix, title, 36, -M_PI, M_PI);
    me_muon_phi_.at(key)->setAxisTitle(phi_x_title);
    me_muon_phi_matched_[key] = bookNumerator1D(ibooker, me_muon_phi_.at(key));
  }  // station
}

void GEMEfficiencyAnalyzer::bookEfficiencyChamber(DQMStore::IBooker& ibooker, const edm::ESHandle<GEMGeometry>& gem) {
  // TODO Efficiency/Source
  ibooker.setCurrentFolder(kFolder_ + "/Efficiency");

  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();

    const std::vector<const GEMSuperChamber*>&& superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs" << std::endl;
      return;
    }

    const int num_chambers = superchambers.size();
    for (const GEMChamber* chamber : superchambers.at(0)->chambers()) {
      const int layer_id = chamber->id().layer();

      const TString&& name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
      const TString&& title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
      const GEMDetId&& key = getReStLaKey(chamber->id());

      me_chamber_[key] =
          ibooker.book1D("chamber" + name_suffix, kMuonName_.c_str() + title_suffix, num_chambers, 0.5, num_chambers + 0.5);
      me_chamber_.at(key)->setAxisTitle("Chamber");
      me_chamber_.at(key)->getTH1F()->SetNdivisions(-num_chambers, "Y");
      for (int binx = 1; binx <= num_chambers; binx++) {
        me_chamber_.at(key)->setBinLabel(binx, std::to_string(binx));
      }

      me_chamber_matched_[key] = bookNumerator1D(ibooker, me_chamber_.at(key));
    }  // layer
  }    // station
}

void GEMEfficiencyAnalyzer::bookEfficiencyEtaPartition(DQMStore::IBooker& ibooker,
                                                       const edm::ESHandle<GEMGeometry>& gem) {
  // TODO Efficiency/Source
  ibooker.setCurrentFolder(kFolder_ + "/Efficiency");

  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();

    const std::vector<const GEMSuperChamber*>&& superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs" << std::endl;
      return;
    }

    for (const GEMChamber* chamber : superchambers.at(0)->chambers()) {
      const int layer_id = chamber->id().layer();
      const int num_ieta = chamber->nEtaPartitions();

      const TString&& name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
      const TString&& title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
      const GEMDetId&& key = getReStLaKey(chamber->id());

      me_ieta_[key] = ibooker.book1D("ieta" + name_suffix, kMuonName_.c_str() + title_suffix, num_ieta, 0.5, num_ieta + 0.5);
      me_ieta_.at(key)->setAxisTitle("i#eta");
      me_ieta_.at(key)->getTH1F()->SetNdivisions(-num_ieta, "Y");
      for (int binx = 1; binx <= num_ieta; binx++) {
        me_ieta_.at(key)->setBinLabel(binx, std::to_string(binx));
      }

      me_ieta_matched_[key] = bookNumerator1D(ibooker, me_ieta_.at(key));
    }  // layer
  }    // station
}

void GEMEfficiencyAnalyzer::bookEfficiencyDetector(DQMStore::IBooker& ibooker, const edm::ESHandle<GEMGeometry>& gem) {
  // TODO Efficiency/Source
  ibooker.setCurrentFolder(kFolder_ + "/Efficiency");

  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();

    const std::vector<const GEMSuperChamber*>&& superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs" << std::endl;
      return;
    }

    const int num_ch = superchambers.size();

    for (const GEMChamber* chamber : superchambers.at(0)->chambers()) {
      const int layer_id = chamber->id().layer();
      const int num_ieta = chamber->nEtaPartitions();

      const TString&& name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
      const TString&& title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
      const GEMDetId&& key = getReStLaKey(chamber->id());

      me_detector_[key] = ibooker.book2D("detector" + name_suffix,
                                         kMuonName_.c_str() + title_suffix,
                                         num_ch,
                                         0.5,
                                         num_ch + 0.5,
                                         num_ieta,
                                         0.5,
                                         num_ieta + 0.5);
      setDetLabelsEta(me_detector_.at(key), station);

      me_detector_matched_[key] = bookNumerator2D(ibooker, me_detector_.at(key));
    }  // layer
  }    // station
}

void GEMEfficiencyAnalyzer::bookResolution(DQMStore::IBooker& ibooker, const edm::ESHandle<GEMGeometry>& gem) {
  ibooker.setCurrentFolder(kFolder_ + "/Resolution");
  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();

    const std::vector<const GEMSuperChamber*>&& superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs" << std::endl;
      return;
    }

    const std::vector<const GEMChamber*>& chambers = superchambers.at(0)->chambers();
    if (not checkRefs(chambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMChamber ptrs" << std::endl;
      return;
    }

    for (const GEMEtaPartition* eta_partition : chambers.at(0)->etaPartitions()) {
      const int ieta = eta_partition->id().roll();

      const GEMDetId&& key = getReStEtKey(eta_partition->id());
      // TODO
      const TString&& name_suffix = TString::Format("_GE%+.2d_R%d", region_id * (station_id * 10 + 1), ieta);
      const TString&& title =
          kMuonName_.c_str() + TString::Format(" : GE%+.2d Roll %d", region_id * (station_id * 10 + 1), ieta);

      me_residual_rphi_[key] =
          ibooker.book1D("residual_rphi" + name_suffix, title, 50, -kResidualRPhiCut_, kResidualRPhiCut_);
      me_residual_rphi_.at(key)->setAxisTitle("Residual in R#phi [cm]");

      me_residual_y_[key] = ibooker.book1D("residual_y" + name_suffix, title, 60, -12.0, 12.0);
      me_residual_y_.at(key)->setAxisTitle("Residual in Local Y [cm]");

      me_pull_y_[key] = ibooker.book1D("pull_y" + name_suffix, title, 60, -3.0, 3.0);
      me_pull_y_.at(key)->setAxisTitle("Pull in Local Y");
    }  // ieta
  }    // station
}

void GEMEfficiencyAnalyzer::bookMisc(DQMStore::IBooker& ibooker, const edm::ESHandle<GEMGeometry>& gem) {
  ibooker.setCurrentFolder(kFolder_ + "/Misc");
  // FIXME the range shoule be dependent on the scenario
  me_prop_r_err_ = ibooker.book1D("prop_r_err", ";Propagation Global R Error [cm];Entries", 20, 0.0, 20.0);
  me_prop_phi_err_ = ibooker.book1D("prop_phi_err", "Propagation Global Phi Error [rad];Entries", 20, 0.0, M_PI);
  me_all_abs_residual_rphi_ = ibooker.book1D("all_abs_residual_rphi", ";Residual in R#phi [cm];Entries", 20, 0.0, 20.0);

  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();

    const std::vector<const GEMSuperChamber*>&& superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs" << std::endl;
      return;
    }
    // ignore layer ids
    const int num_ch = superchambers.size();

    const GEMDetId&& key = getReStKey(region_id, station_id);
    const TString&& name_suffix = GEMUtils::getSuffixName(region_id, station_id);
    const TString&& title_suffix = GEMUtils::getSuffixTitle(region_id, station_id);
    me_prop_chamber_[key] = ibooker.book1D("prop_chamber" + name_suffix, title_suffix, num_ch, 0.5, num_ch + 0.5);
    me_prop_chamber_.at(key)->setAxisTitle("Destination Chamber Id", 1);
    me_prop_chamber_.at(key)->setAxisTitle("Entries", 2);
  }  // station
}

void GEMEfficiencyAnalyzer::analyze(const edm::Event& event, const edm::EventSetup& setup) {
  if (kMuonTrackType_ == reco::Muon::MuonTrackType::None) {
    return;
  }

  edm::Handle<GEMRecHitCollection> rechit_collection;
  event.getByToken(kGEMRecHitToken_, rechit_collection);
  if (not rechit_collection.isValid()) {
    edm::LogError(kLogCategory_) << "GEMRecHitCollection is invalid" << std::endl;
    return;
  }

  edm::Handle<edm::View<reco::Muon> > muon_view;
  event.getByToken(kMuonToken_, muon_view);
  if (not muon_view.isValid()) {
    edm::LogError(kLogCategory_) << "View<Muon> is invalid" << std::endl;
    return;
  }

  edm::ESHandle<GEMGeometry> gem = setup.getHandle(kGEMToken_);
  if (not gem.isValid()) {
    edm::LogError(kLogCategory_) << "GEMGeometry is invalid" << std::endl;
    return;
  }

  edm::ESHandle<GlobalTrackingGeometry> global_tracking_geometry = setup.getHandle(kGlobalTrackingGeometryToken_);
  if (not global_tracking_geometry.isValid()) {
    edm::LogError(kLogCategory_) << "GlobalTrackingGeometry is invalid" << std::endl;
    return;
  }

  edm::ESHandle<TransientTrackBuilder> transient_track_builder = setup.getHandle(kTransientTrackBuilderToken_);
  if (not transient_track_builder.isValid()) {
    edm::LogError(kLogCategory_) << "TransientTrackRecord is invalid" << std::endl;
    return;
  }

  muon_service_->update(setup);
  edm::ESHandle<Propagator>&& propagator = muon_service_->propagator("SteppingHelixPropagatorAny");
  if (not propagator.isValid()) {
    edm::LogError(kLogCategory_) << "Propagator is invalid" << std::endl;
    return;
  }

  if (muon_view->empty()) {
    edm::LogInfo(kLogCategory_) << "empty muon collection" << std::endl;
    return;
  }

  const std::vector<GEMLayerData>&& layer_vector = buildGEMLayers(gem);

  for (const reco::Muon& muon : *muon_view) {
    const reco::Track* track = muon.muonTrack(kMuonTrackType_).get();
    if (track == nullptr) {
      edm::LogError(kLogCategory_) << "failed to get a muon track" << std::endl;
      continue;
    }

    const reco::TransientTrack&& transient_track = transient_track_builder->build(track);
    if (not transient_track.isValid()) {
      edm::LogError(kLogCategory_) << "failed to build TransientTrack" << std::endl;
      continue;
    }

    for (const GEMLayerData& layer : layer_vector) {
      if (kUseSkipLayer_ and skipLayer(track, layer)) {
        edm::LogInfo(kLogCategory_) << "skip GEM Layer" << std::endl;
        continue;
      }

      const auto&& [start_state, start_id] = getStartingState(transient_track, layer, global_tracking_geometry);

      if (not start_state.isValid()) {
        edm::LogInfo(kLogCategory_) << "failed to get a starting state" << std::endl;
        continue;
      }

      if (kUseOnlyME11_ and (not isME11(start_id))) {
        edm::LogInfo(kLogCategory_) << "skip a starting state because it is not ME11" << std::endl;
        continue;
      }

      // the trajectory state on the destination surface
      const TrajectoryStateOnSurface&& dest_state = propagator->propagate(start_state, *(layer.disk));
      if (not dest_state.isValid()) {
        edm::LogInfo(kLogCategory_) << "failed to propagate a muon" << std::endl;
        continue;
      }

      const GlobalPoint&& dest_global_pos = dest_state.globalPosition();

      if (not checkBounds(dest_global_pos, (*layer.disk))) {
        edm::LogInfo(kLogCategory_) << "failed to pass checkBounds" << std::endl;
        continue;
      }

      const GEMEtaPartition* eta_partition = findEtaPartition(dest_global_pos, layer.chambers);
      if (eta_partition == nullptr) {
        edm::LogInfo(kLogCategory_) << "failed to find an eta partition" << std::endl;
        continue;
      }

      const BoundPlane& surface = eta_partition->surface();

      const LocalPoint&& dest_local_pos = eta_partition->toLocal(dest_global_pos);
      const LocalError&& dest_local_err = dest_state.localError().positionError();
      const GlobalError& dest_global_err = ErrorFrameTransformer().transform(dest_local_err, surface);

      const double dest_global_r_err = std::sqrt(dest_global_err.rerr(dest_global_pos));
      const double dest_global_phi_err = std::sqrt(dest_global_err.phierr(dest_global_pos));

      const GEMDetId&& gem_id = eta_partition->id();
      const GEMDetId&& rs_key = getReStKey(gem_id);
      const GEMDetId&& rsl_key = getReStLaKey(gem_id);
      const GEMDetId&& rse_key = getReStEtKey(gem_id);

      // FIXME clever way to clamp values?
      fillMEWithinLimits(me_prop_r_err_, dest_global_r_err);
      fillMEWithinLimits(me_prop_phi_err_, dest_global_phi_err);
      me_prop_chamber_.at(rs_key)->Fill(gem_id.chamber());

      if (kUsePropRErrorCut_ and (dest_global_r_err > kPropRErrorCut_)) {
        edm::LogInfo(kLogCategory_) << "failed to pass a propagation global R error cut" << std::endl;
        continue;
      }

      if (kUsePropPhiErrorCut_ and (dest_global_phi_err > kPropPhiErrorCut_)) {
        edm::LogInfo(kLogCategory_) << "failed to pass a propagation global phi error cut" << std::endl;
        continue;
      }

      const int chamber_id = gem_id.chamber();
      const int ieta = gem_id.ieta();

      const double muon_pt = muon.pt();
      const double muon_eta = std::fabs(muon.eta());
      const double muon_phi = muon.phi();

      fillMEWithinLimits(me_muon_pt_, rs_key, muon_pt);
      fillMEWithinLimits(me_muon_eta_, rs_key, muon_eta);
      fillME(me_muon_phi_, rs_key, muon_phi);

      fillME(me_chamber_, rsl_key, chamber_id);
      fillME(me_ieta_, rsl_key, ieta);
      fillME(me_detector_, rsl_key, chamber_id, ieta);

      const auto&& [hit, residual_rphi] = findClosetHit(dest_global_pos, rechit_collection->get(gem_id), eta_partition);

      if (hit == nullptr) {
        edm::LogInfo(kLogCategory_) << "failed to find a hit" << std::endl;
        continue;
      }

      fillMEWithinLimits(me_all_abs_residual_rphi_, std::abs(residual_rphi));
      if (std::abs(residual_rphi) > kResidualRPhiCut_) {
        edm::LogInfo(kLogCategory_) << "failed to pass the residual rphi cut" << std::endl;
        continue;
      }

      fillMEWithinLimits(me_muon_pt_matched_, rs_key, muon_pt);
      fillMEWithinLimits(me_muon_eta_matched_, rs_key, muon_eta);
      fillME(me_muon_phi_matched_, rs_key, muon_phi);

      fillME(me_chamber_matched_, rsl_key, chamber_id);
      fillME(me_ieta_matched_, rsl_key, ieta);
      fillME(me_detector_matched_, rsl_key, chamber_id, ieta);

      const LocalPoint&& hit_local_pos = hit->localPosition();
      const LocalError&& hit_local_err = hit->localPositionError();

      const float residual_y = dest_local_pos.y() - hit_local_pos.y();
      const float pull_y = residual_y / std::sqrt(dest_local_err.yy() + hit_local_err.yy());

      fillME(me_residual_rphi_, rse_key, residual_rphi);
      fillME(me_residual_y_, rse_key, residual_y);
      fillME(me_pull_y_, rse_key, pull_y);
    }  // layer
  }    // Muon
}

reco::Muon::MuonTrackType GEMEfficiencyAnalyzer::getMuonTrackType(const std::string& name) {
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
                                 << ". All events will be skipped." << std::endl;
  }

  return muon_track_type;
}

std::vector<GEMEfficiencyAnalyzer::GEMLayerData> GEMEfficiencyAnalyzer::buildGEMLayers(
    const edm::ESHandle<GEMGeometry>& gem) {
  std::vector<GEMLayerData> layer_vector;

  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();
    const bool is_ge11 = station_id == 1;

    // (layer_id, is_odd) - chambers
    std::map<std::pair<int, bool>, std::vector<const GEMChamber*> > chambers_per_layer;

    for (const GEMSuperChamber* super_chamber : station->superChambers()) {
      // For GE0 and GE21, 'is_odd' is always set to 'false'
      const bool is_odd = is_ge11 ? super_chamber->id().chamber() % 2 == 1 : false;

      for (const GEMChamber* chamber : super_chamber->chambers()) {
        const int layer_id = chamber->id().layer();
        const std::pair<int, bool> key{layer_id, is_odd};

        if (chambers_per_layer.find(key) == chambers_per_layer.end())
          chambers_per_layer.insert({key, std::vector<const GEMChamber*>()});

        chambers_per_layer.at(key).push_back(chamber);
      }  // GEMChamber
    }    // GEMSuperChamber

    for (auto [key, chamber_vector] : chambers_per_layer) {
      const int layer_id = key.first;

      // chambers should have same R and Z spans.
      auto [rmin, rmax] = chamber_vector.at(0)->surface().rSpan();
      auto [zmin, zmax] = chamber_vector.at(0)->surface().zSpan();

      // layer position and rotation
      const float layer_z = chamber_vector.at(0)->position().z();
      Surface::PositionType position{0.f, 0.f, layer_z};
      Surface::RotationType rotation;

      zmin -= layer_z;
      zmax -= layer_z;

      // the bounds from min and max R and Z in the local coordinates.
      SimpleDiskBounds* bounds = new SimpleDiskBounds(rmin, rmax, zmin, zmax);
      const Disk::DiskPointer&& layer = Disk::build(position, rotation, bounds);

      layer_vector.emplace_back(layer, chamber_vector, region_id, station_id, layer_id);
    }  // layer
  }    // GEMStation

  return layer_vector;
}

bool GEMEfficiencyAnalyzer::areSameLayer(const GEMEfficiencyAnalyzer::GEMLayerData& layer, const uint32_t raw_id) {
  if (not MuonHitHelper::isGEM(raw_id)) return false;

  const GEMDetId gem_id{raw_id};
  if (layer.region != gem_id.region()) return false;
  if (layer.station != gem_id.station()) return false;

  return (layer.layer == gem_id.layer());
}

std::pair<TrajectoryStateOnSurface, DetId> GEMEfficiencyAnalyzer::getStartingState(
    const reco::TransientTrack& transient_track,
    const GEMLayerData& layer,
    const edm::ESHandle<GlobalTrackingGeometry>& geometry) {
  TrajectoryStateOnSurface starting_state;
  DetId starting_id;

  if (kMuonTrackType_ == reco::Muon::MuonTrackType::CombinedTrack) {
    std::tie(starting_state, starting_id) = findStartingState(transient_track, layer, geometry);

  } else if (kMuonTrackType_ == reco::Muon::MuonTrackType::OuterTrack) {
    const reco::Track& track = transient_track.track();
    const bool is_insideout = isInsideOut(track);

    const DetId inner_id{(is_insideout ? track.outerDetId() : track.innerDetId())};

    if (areSameLayer(layer, inner_id.rawId())) {
      std::tie(starting_state, starting_id) = findStartingState(transient_track, layer, geometry);

    } else {
      starting_id = inner_id;
      if (is_insideout)
        starting_state = transient_track.outermostMeasurementState();
      else
        starting_state = transient_track.innermostMeasurementState();
    }

  } else if (kMuonTrackType_ == reco::Muon::MuonTrackType::InnerTrack) {
    const reco::Track& track = transient_track.track();
    const bool is_insideout = isInsideOut(track);

    starting_id = DetId{(is_insideout ? track.innerDetId() : track.outerDetId())};

    if (is_insideout)
      starting_state = transient_track.innermostMeasurementState();
    else
      starting_state = transient_track.outermostMeasurementState();
  }


  return std::make_pair(starting_state, starting_id);
}

std::pair<TrajectoryStateOnSurface, DetId> GEMEfficiencyAnalyzer::findStartingState(
    const reco::TransientTrack& transient_track,
    const GEMLayerData& layer,
    const edm::ESHandle<GlobalTrackingGeometry>& geometry) {
  GlobalPoint starting_point;
  DetId starting_id;
  float min_distance = 1e12;
  bool found = false;

  // TODO optimize this loop because hits should be ordered..
  for (auto rechit = transient_track.recHitsBegin(); rechit != transient_track.recHitsEnd(); rechit++) {
    const DetId&& det_id = (*rechit)->geographicalId();

    if (areSameLayer(layer, det_id.rawId())) {
      continue;
    }

    const GeomDet* det = geometry->idToDet(det_id);
    const GlobalPoint&& global_position = det->toGlobal((*rechit)->localPosition());
    const float distance = std::abs(layer.disk->localZclamped(global_position));
    if (distance < min_distance) {
      found = true;
      min_distance = distance;
      starting_point = global_position;
      starting_id = det_id;
    }
  }

  TrajectoryStateOnSurface starting_state;
  if (found) {
    starting_state = transient_track.stateOnSurface(starting_point);
  }
  return std::make_pair(starting_state, starting_id);
}

bool GEMEfficiencyAnalyzer::isME11(const DetId& det_id) {
  if (not MuonHitHelper::isCSC(det_id))
    return false;
  const CSCDetId csc_id{det_id};
  return (csc_id.station() == 1) and ((csc_id.ring() == 1) or (csc_id.ring() == 4));
}

bool GEMEfficiencyAnalyzer::skipLayer(const reco::Track* track, const GEMLayerData& layer) {
  const bool is_same_region = track->eta() * layer.region > 0;

  bool skip = false;
  if (kIsCosmics_) {
    float p2_in = track->innerMomentum().mag2();
    float p2_out = track->outerMomentum().mag2();
    if (isInsideOut(*track))
      std::swap(p2_in, p2_out);
    const bool is_outgoing = p2_in > p2_out;

    skip = (is_outgoing xor is_same_region);

  } else {
    // beam scenario
    skip = not is_same_region;
  }

  return skip;
}

bool GEMEfficiencyAnalyzer::checkBounds(const GlobalPoint& global_point, const Plane& plane) {
  const LocalPoint&& local_point = plane.toLocal(global_point);
  const LocalPoint local_point_2d(local_point.x(), local_point.y(), 0.0f);
  return plane.bounds().inside(local_point_2d);
}

const GEMEtaPartition* GEMEfficiencyAnalyzer::findEtaPartition(const GlobalPoint& global_point,
                                                               const std::vector<const GEMChamber*>& chamber_vector) {
  const GEMEtaPartition* bound = nullptr;
  for (const GEMChamber* chamber : chamber_vector) {
    if (not checkBounds(global_point, chamber->surface()))
      continue;

    for (const GEMEtaPartition* eta_partition : chamber->etaPartitions()) {
      if (checkBounds(global_point, eta_partition->surface())) {
        bound = eta_partition;
        break;
      }
    }  // GEMEtaPartition
  }    // GEMChamber

  return bound;
}

std::pair<const GEMRecHit*, float> GEMEfficiencyAnalyzer::findClosetHit(const GlobalPoint& dest_global_pos,
                                                                        const GEMRecHitCollection::range& range,
                                                                        const GEMEtaPartition* eta_partition) {
  const StripTopology& topology = eta_partition->specificTopology();
  const LocalPoint&& dest_local_pos = eta_partition->toLocal(dest_global_pos);
  const float dest_local_x = dest_local_pos.x();
  const float dest_local_y = dest_local_pos.y();

  const GEMRecHit* closest_hit = nullptr;
  float min_residual_rphi = 1e6;

  for (auto hit = range.first; hit != range.second; ++hit) {
    const LocalPoint&& hit_local_pos = hit->localPosition();
    const float hit_local_phi = topology.stripAngle(eta_partition->strip(hit_local_pos));

    const float residual_x = dest_local_x - hit_local_pos.x();
    const float residual_y = dest_local_y - hit_local_pos.y();
    const float residual_rphi = std::cos(hit_local_phi) * residual_x + std::sin(hit_local_phi) * residual_y;

    if (std::abs(residual_rphi) < std::abs(min_residual_rphi)) {
      min_residual_rphi = residual_rphi;
      closest_hit = &(*hit);
    }
  }

  return std::make_pair(closest_hit, min_residual_rphi);
}
