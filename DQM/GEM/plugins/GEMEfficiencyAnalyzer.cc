#include "DQM/GEM/plugins/GEMEfficiencyAnalyzer.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Utilities/interface/Likely.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/ErrorFrameTransformer.h"
#include "DataFormats/GeometrySurface/interface/SimpleDiskBounds.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"
#include "Validation/MuonHits/interface/MuonHitHelper.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"

GEMEfficiencyAnalyzer::GEMEfficiencyAnalyzer(const edm::ParameterSet& ps)
    : GEMOfflineDQMBase(ps),
      kGEMToken_(esConsumes<GEMGeometry, MuonGeometryRecord>()),
      kGEMTokenBeginRun_(esConsumes<edm::Transition::BeginRun>()),
      kGlobalTrackingGeometryToken_(esConsumes<GlobalTrackingGeometry, GlobalTrackingGeometryRecord>()),
      kTrasientTrackToken_(esConsumes<TransientTrackBuilder, TransientTrackRecord>(edm::ESInputTag("", "TransientTrackBuilder"))),
      kRecHitToken_(consumes<GEMRecHitCollection>(ps.getParameter<edm::InputTag>("recHitTag"))),
      kMuonToken_(consumes<edm::View<reco::Muon> >(ps.getParameter<edm::InputTag>("muonTag"))),
      kMuonTrackType_(getMuonTrackType(ps.getUntrackedParameter<std::string>("muonTrackType"))),
      kName_(ps.getUntrackedParameter<std::string>("name")),
      kFolder_(ps.getUntrackedParameter<std::string>("folder")),
      kIsCosmics_(ps.getUntrackedParameter<bool>("isCosmics")),
      kMuonPtMin_(static_cast<float>(ps.getUntrackedParameter<double>("muonPtMin"))),
      kMuonAbsEtaMin_(static_cast<float>(ps.getUntrackedParameter<double>("muonAbsEtaMin"))),
      kMuonAbsEtaMax_(static_cast<float>(ps.getUntrackedParameter<double>("muonAbsEtaMax"))),
      kUseSkipLayer_(ps.getUntrackedParameter<bool>("useSkipLayer")),
      kUseOnlyME11_(ps.getUntrackedParameter<bool>("useOnlyME11")), // FIXME
      kPropagationRErrorCut_(static_cast<float>(ps.getUntrackedParameter<double>("propagationRErrorCut"))),
      kPropagationPhiErrorCut_(static_cast<float>(ps.getUntrackedParameter<double>("propagationPhiErrorCut"))),
      kBoundsErrorScale_(static_cast<float>(ps.getUntrackedParameter<double>("boundsErrorScale"))),
      kResidualRPhiCut_(static_cast<float>(ps.getUntrackedParameter<double>("residualRPhiCut"))),
      kPtBins_(ps.getUntrackedParameter<std::vector<double> >("ptBins")),
      kEtaNBins_(ps.getUntrackedParameter<int>("etaNbins")),
      kEtaLow_(ps.getUntrackedParameter<double>("etaLow")),
      kEtaUp_(ps.getUntrackedParameter<double>("etaUp")) {

  muon_service_ = std::make_unique<MuonServiceProxy>(
      ps.getParameter<edm::ParameterSet>("ServiceParameters"),
      consumesCollector());
}

GEMEfficiencyAnalyzer::~GEMEfficiencyAnalyzer() {}

void GEMEfficiencyAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("recHitTag", edm::InputTag("gemRecHits"));
  desc.add<edm::InputTag>("muonTag", edm::InputTag("muons"));
  desc.addUntracked<std::string>("muonTrackType", "CombinedTrack");
  desc.addUntracked<std::string>("name", "Global Muon");
  desc.addUntracked<std::string>("folder", "GEM/Efficiency/type0");
  desc.addUntracked<bool>("isCosmics", false);
  desc.addUntracked<double>("muonPtMin", 20.0f);
  desc.addUntracked<double>("muonAbsEtaMin", 1.4);
  desc.addUntracked<double>("muonAbsEtaMax", 2.3);
  desc.addUntracked<bool>("useSkipLayer", true);
  desc.addUntracked<bool>("useOnlyME11", true); // FIXME
  desc.addUntracked<double>("propagationRErrorCut", 0.5); // cm
  desc.addUntracked<double>("propagationPhiErrorCut", 0.1); // degree
  desc.addUntracked<double>("boundsErrorScale", -2);
  desc.addUntracked<double>("residualRPhiCut", 2.0);  // TODO need to be tuned
  desc.addUntracked<std::vector<double> >("ptBins", {0., 5., 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., 120.});
  desc.addUntracked<int>("etaNbins", 20);
  desc.addUntracked<double>("etaLow", 1.0);
  desc.addUntracked<double>("etaUp", 3.0);
  {
    edm::ParameterSetDescription psd0;
    psd0.setAllowAnything();
    desc.add<edm::ParameterSetDescription>("ServiceParameters", psd0);
  }

  descriptions.add("gemEfficiencyAnalyzerDefault", desc);
}

void GEMEfficiencyAnalyzer::bookHistograms(DQMStore::IBooker& ibooker,
                                           edm::Run const& run,
                                           edm::EventSetup const& setup) {
  const edm::ESHandle<GEMGeometry> gem = setup.getHandle(kGEMTokenBeginRun_);
  if (not gem.isValid()) {
    edm::LogError(kLogCategory_) << "invalid GEMGeometry";
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

    const TString&& title = kName_.c_str() + title_suffix;

    TH1F* h_muon_pt = new TH1F("muon_pt" + name_suffix, title, pt_nbinsx, &kPtBins_[0]);
    h_muon_pt->SetXTitle(pt_x_title);
    me_muon_pt_[key] = ibooker.book1D(h_muon_pt->GetName(), h_muon_pt);
    me_muon_pt_matched_[key] = bookNumerator1D(ibooker, me_muon_pt_[key]);

    me_muon_eta_[key] = ibooker.book1D("muon_eta" + name_suffix, title, kEtaNBins_, kEtaLow_, kEtaUp_);
    me_muon_eta_[key]->setXTitle(eta_x_title);
    me_muon_eta_matched_[key] = bookNumerator1D(ibooker, me_muon_eta_[key]);

    me_muon_phi_[key] = ibooker.book1D("muon_phi" + name_suffix, title, 36, -M_PI, M_PI);
    me_muon_phi_[key]->setAxisTitle(phi_x_title);
    me_muon_phi_matched_[key] = bookNumerator1D(ibooker, me_muon_phi_[key]);
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
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs";
      return;
    }

    const int num_chambers = superchambers.size();
    for (const GEMChamber* chamber : superchambers.front()->chambers()) {
      const int layer_id = chamber->id().layer();

      const TString&& name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
      const TString&& title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
      const GEMDetId&& key = getReStLaKey(chamber->id());

      me_chamber_[key] =
          ibooker.book1D("chamber" + name_suffix, kName_.c_str() + title_suffix, num_chambers, 0.5, num_chambers + 0.5);
      me_chamber_[key]->setAxisTitle("Chamber");
      me_chamber_[key]->getTH1F()->SetNdivisions(-num_chambers, "Y");
      for (int binx = 1; binx <= num_chambers; binx++) {
        me_chamber_[key]->setBinLabel(binx, std::to_string(binx));
      }

      me_chamber_matched_[key] = bookNumerator1D(ibooker, me_chamber_[key]);
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
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs";
      return;
    }

    for (const GEMChamber* chamber : superchambers.front()->chambers()) {
      const int layer_id = chamber->id().layer();
      const int num_ieta = chamber->nEtaPartitions();

      const TString&& name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
      const TString&& title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
      const GEMDetId&& key = getReStLaKey(chamber->id());

      me_ieta_[key] = ibooker.book1D("ieta" + name_suffix, kName_.c_str() + title_suffix, num_ieta, 0.5, num_ieta + 0.5);
      me_ieta_[key]->setAxisTitle("i#eta");
      me_ieta_[key]->getTH1F()->SetNdivisions(-num_ieta, "Y");
      for (int binx = 1; binx <= num_ieta; binx++) {
        me_ieta_[key]->setBinLabel(binx, std::to_string(binx));
      }

      me_ieta_matched_[key] = bookNumerator1D(ibooker, me_ieta_[key]);
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
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs";
      return;
    }

    const int num_ch = superchambers.size();

    for (const GEMChamber* chamber : superchambers.front()->chambers()) {
      const int layer_id = chamber->id().layer();
      const int num_ieta = chamber->nEtaPartitions();

      const TString&& name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
      const TString&& title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
      const GEMDetId&& key = getReStLaKey(chamber->id());

      me_detector_[key] = ibooker.book2D("detector" + name_suffix,
                                         kName_.c_str() + title_suffix,
                                         num_ch,
                                         0.5,
                                         num_ch + 0.5,
                                         num_ieta,
                                         0.5,
                                         num_ieta + 0.5);
      setDetLabelsEta(me_detector_[key], station);

      me_detector_matched_[key] = bookNumerator2D(ibooker, me_detector_[key]);
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
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs";
      return;
    }

    const std::vector<const GEMChamber*>& chambers = superchambers.front()->chambers();
    if (not checkRefs(chambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMChamber ptrs";
      return;
    }

    for (const GEMEtaPartition* eta_partition : chambers.front()->etaPartitions()) {
      const int ieta = eta_partition->id().ieta();

      const GEMDetId&& key = getReStEtKey(eta_partition->id());
      // TODO
      const TString&& name_suffix = TString::Format("_GE%d1-%s-E%d", station_id, (region_id > 0 ? "P" : "M"), ieta);
      const TString&& title =
          kName_.c_str() + TString::Format(" GE%d1-%s-E%d", station_id, (region_id > 0 ? "P" : "M"), ieta);

      me_residual_rphi_[key] =
          ibooker.book1D("residual_rphi" + name_suffix, title, 50, -kResidualRPhiCut_, kResidualRPhiCut_);
      me_residual_rphi_[key]->setAxisTitle("Residual in R#phi [cm]");

    }  // ieta
  }    // station
}

void GEMEfficiencyAnalyzer::bookMisc(DQMStore::IBooker& ibooker, const edm::ESHandle<GEMGeometry>& gem) {
  ibooker.setCurrentFolder(kFolder_ + "/Misc");
  // FIXME the range shoule be dependent on the scenario
  me_prop_r_err_ = ibooker.book1D("prop_r_err", ";Propagation Global R Error [cm];Entries", 10, 0.0, 5.0);
  me_prop_phi_err_ = ibooker.book1D("prop_phi_err", "Propagation Global Phi Error [deg];Entries", 30, 0.0, 30.0);
  me_all_abs_residual_rphi_ = ibooker.book1D("all_abs_residual_rphi", ";Residual in R#phi [cm];Entries", 10, 0.0, 10.0);

  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();

    const std::vector<const GEMSuperChamber*>&& superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs";
      return;
    }
    // ignore layer ids
    const int num_ch = superchambers.size();

    const GEMDetId&& key = getReStKey(region_id, station_id);
    const TString&& name_suffix = GEMUtils::getSuffixName(region_id, station_id);
    const TString&& title_suffix = GEMUtils::getSuffixTitle(region_id, station_id);
    me_prop_chamber_[key] = ibooker.book1D("prop_chamber" + name_suffix, title_suffix, num_ch, 0.5, num_ch + 0.5);
    me_prop_chamber_[key]->setAxisTitle("Destination Chamber Id", 1);
    me_prop_chamber_[key]->setAxisTitle("Entries", 2);

  }  // station

  // FIXME debug
  ibooker.setCurrentFolder(kFolder_ + "/Efficiency");

  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();
    const GEMDetId&& key = getReStKey(region_id, station_id);
    const TString&& name_suffix = GEMUtils::getSuffixName(region_id, station_id);
    const TString&& title_suffix = GEMUtils::getSuffixTitle(region_id, station_id);

    me_prop_err_r_[key] = ibooker.book1D("prop_err_r" + name_suffix, title_suffix, 200, 0.0, 2.0);
    me_prop_err_r_matched_[key] = bookNumerator1D(ibooker, me_prop_err_r_[key]);

    me_prop_err_phi_[key] = ibooker.book1D("prop_err_phi" + name_suffix, title_suffix, 100, 0.0, 1.0);
    me_prop_err_phi_matched_[key] = bookNumerator1D(ibooker, me_prop_err_phi_[key]);
  }
}

void GEMEfficiencyAnalyzer::analyze(const edm::Event& event, const edm::EventSetup& setup) {
  if UNLIKELY (kMuonTrackType_ == reco::Muon::MuonTrackType::None) {
    return;
  }

  /////////////////////////////////////////////////////////////////////////////
  const edm::Handle<GEMRecHitCollection> rechit_collection = event.getHandle(kRecHitToken_);
  if (not rechit_collection.isValid()) {
    edm::LogError(kLogCategory_) << "invalid GEMRecHitCollection";
    return;
  }

  const edm::Handle<edm::View<reco::Muon> > muon_view = event.getHandle(kMuonToken_);
  if (not muon_view.isValid()) {
    edm::LogError(kLogCategory_) << "invalid edm::View<reco::Muon>";
    return;
  }

  /////////////////////////////////////////////////////////////////////////////
  const edm::ESHandle<GEMGeometry> gem = setup.getHandle(kGEMToken_);
  if (not gem.isValid()) {
    edm::LogError(kLogCategory_) << "invalid GEMGeometry";
    return;
  }

  const edm::ESHandle<GlobalTrackingGeometry> global_tracking_geometry = setup.getHandle(kGlobalTrackingGeometryToken_);
  if (not global_tracking_geometry.isValid()) {
    edm::LogError(kLogCategory_) << "invalid GlobalTrackingGeometry";
    return;
  }

  const edm::ESHandle<TransientTrackBuilder> transient_track_builder = setup.getHandle(kTrasientTrackToken_);
  if (not transient_track_builder.isValid()) {
    edm::LogError(kLogCategory_) << "invalid TransientTrackRecord";
    return;
  }

  muon_service_->update(setup);
  const edm::ESHandle<Propagator> propagator = muon_service_->propagator("SteppingHelixPropagatorAny");
  if (not propagator.isValid()) {
    edm::LogError(kLogCategory_) << "invalid Propagator";
    return;
  }

  //////////////////////////////////////////////////////////////////////////////
  if (muon_view->empty()) {
    LogDebug(kLogCategory_) << "empty muon collection";
    return;
  }

  //////////////////////////////////////////////////////////////////////////////
  const std::vector<GEMLayerData>&& layer_vector = buildGEMLayers(gem);

  for (const reco::Muon& muon : *muon_view) {
    const reco::Track* track = muon.muonTrack(kMuonTrackType_).get();
    if (track == nullptr) {
      LogDebug(kLogCategory_) << "failed to get a reco::Track from the reco::Muon";
      continue;
    }

    const reco::TransientTrack&& transient_track = transient_track_builder->build(track);
    if (not transient_track.isValid()) {
      edm::LogError(kLogCategory_) << "failed to build TransientTrack";
      continue;
    }

    for (const GEMLayerData& layer : layer_vector) {
      if (kUseSkipLayer_ and skipLayer(track, layer)) {
        // TODO print layer id and the direction of muon.
        LogDebug(kLogCategory_) << "skip GEM Layer";
        continue;
      }

      const auto& [start_state, start_id] = findStartingState(transient_track, layer, global_tracking_geometry);
      if (not start_state.isValid()) {
        LogDebug(kLogCategory_) << "failed to get a starting state";
        continue;
      }

      if (layer.station == 1) {
        if (kUseOnlyME11_) {
          if (not MuonHitHelper::isCSC(start_id)) {
            LogDebug(kLogCategory_) << "skip a starting state because it is not CSC";
            continue;
          }
          
          const CSCDetId csc_id{start_id};
          if (not csc_id.isME11()) {
            LogDebug(kLogCategory_) << "skip a starting state because it is not ME11";
            continue;
          }
        }
      }

      // the trajectory state on the destination surface
      const TrajectoryStateOnSurface&& propagated_state = propagator->propagate(start_state, *layer.disk);
      if (not propagated_state.isValid()) {
        LogDebug(kLogCategory_) << "failed to propagate a muon";
        continue;
      }

      const GlobalPoint&& prop_global_pos = propagated_state.globalPosition();
      const GlobalError& prop_global_err = ErrorFrameTransformer::transform(
          propagated_state.localError().positionError(), *layer.disk);
      if (not checkBounds(prop_global_pos, prop_global_err, *layer.disk)) {
        LogDebug(kLogCategory_) << "failed to pass checkBounds";
        continue;
      }

      const double prop_global_r_err = std::sqrt(prop_global_err.rerr(prop_global_pos));
      const double prop_global_phi_err = std::sqrt(prop_global_err.phierr(prop_global_pos));
      const double prop_global_phi_err_deg = TMath::RadToDeg() * prop_global_phi_err;

      fillMEWithinLimits(me_prop_r_err_, prop_global_r_err);
      fillMEWithinLimits(me_prop_phi_err_, prop_global_phi_err_deg);
      // FIXME fillME(me_prop_chamber_, rs_key, chamber_id);

      if (prop_global_r_err > kPropagationRErrorCut_) {
        LogDebug(kLogCategory_) << "failed to pass kPropagationRErrorCut_";
        continue;
      }

      if (prop_global_phi_err_deg > kPropagationPhiErrorCut_) {
        LogDebug(kLogCategory_) << "failed to pass kPropagationPhiErrorCut_";
        continue;
      }

      const GEMEtaPartition* eta_partition = findEtaPartition(prop_global_pos, prop_global_err, layer.chambers);
      if (eta_partition == nullptr) {
        LogDebug(kLogCategory_) << "failed to find an eta partition";
        continue;
      }

      const GEMDetId&& gem_id = eta_partition->id();
      const GEMDetId&& rs_key = getReStKey(gem_id);
      const GEMDetId&& rsl_key = getReStLaKey(gem_id);
      const GEMDetId&& rse_key = getReStEtKey(gem_id);
      const int chamber_id = gem_id.chamber();
      const int ieta = gem_id.ieta();
      const double muon_pt = muon.pt();
      const double muon_eta = std::fabs(muon.eta());

      const bool passed_pt_cut = muon_pt > kMuonPtMin_;
      const bool passed_eta_cut = (muon_eta > kMuonAbsEtaMin_) and (muon_eta < kMuonAbsEtaMax_);

      if (passed_eta_cut) {
        fillMEWithinLimits(me_muon_pt_, rs_key, muon_pt);
      }

      if (passed_pt_cut) {
        fillMEWithinLimits(me_muon_eta_, rs_key, muon_eta);
      }

      if (passed_pt_cut and passed_eta_cut) {
        fillME(me_muon_phi_, rs_key, muon.phi());
        fillME(me_chamber_, rsl_key, chamber_id);
        fillME(me_ieta_, rsl_key, ieta);
        fillME(me_detector_, rsl_key, chamber_id, ieta);

        fillMEWithinLimits(me_prop_err_r_, rs_key, prop_global_r_err);
        fillMEWithinLimits(me_prop_err_phi_, rs_key, prop_global_phi_err_deg);
      }

      const auto& [hit, rdphi] = findClosestHit(start_state.globalPosition(),
                                                eta_partition,
                                                rechit_collection,
                                                gem);
      if (hit == nullptr) {
        LogDebug(kLogCategory_) << "failed to find a hit";
        continue;
      }

      fillMEWithinLimits(me_all_abs_residual_rphi_, std::abs(rdphi));
      if (std::abs(rdphi) > kResidualRPhiCut_) {
        LogDebug(kLogCategory_) << "failed to pass the residual rphi cut";
        continue;
      }

      if (passed_eta_cut) {
        fillMEWithinLimits(me_muon_pt_matched_, rs_key, muon_pt);
      }

      if (passed_pt_cut) {
        fillMEWithinLimits(me_muon_eta_matched_, rs_key, muon_eta);
      }

      if (passed_pt_cut and passed_eta_cut) {
        fillME(me_muon_phi_matched_, rs_key, muon.phi());
        fillME(me_chamber_matched_, rsl_key, gem_id.chamber());
        fillME(me_ieta_matched_, rsl_key, gem_id.ieta());
        fillME(me_detector_matched_, rsl_key, gem_id.chamber(), ieta);

        fillMEWithinLimits(me_prop_err_r_matched_, rs_key, prop_global_r_err);
        fillMEWithinLimits(me_prop_err_phi_matched_, rs_key, prop_global_phi_err_deg);
      }

      //////////////////////////////////////////////////////////////////////////
      // Fill resolutions
      const LocalPoint& prop_local_pos = eta_partition->toLocal(prop_global_pos);
      const LocalError& prop_local_err = ErrorFrameTransformer::transform(prop_global_err, eta_partition->surface());
      const LocalPoint& hit_local_pos = hit->localPosition();
      const LocalError& hit_local_err = hit->localPositionError();

      if (passed_pt_cut and passed_eta_cut) {
        fillMEWithinLimits(me_residual_rphi_, rse_key, rdphi);
      }
    }  // layer
  }    // Muon
}

// Returns an reco::Muon::MuonTrackType
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
                                 << ". All events will be skipped.";
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
      auto [rmin, rmax] = chamber_vector.front()->surface().rSpan();
      auto [zmin, zmax] = chamber_vector.front()->surface().zSpan();

      // layer position and rotation
      const float layer_z = chamber_vector.front()->position().z();
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

std::pair<TrajectoryStateOnSurface, DetId> GEMEfficiencyAnalyzer::findStartingState(
    const reco::TransientTrack& transient_track,
    const GEMLayerData& layer,
    const edm::ESHandle<GlobalTrackingGeometry>& geometry) {
  GlobalPoint starting_point;
  DetId start_id;
  float min_distance = 1e12; // any large number
  bool found = false;

  // TODO optimize this loop because hits should be ordered..
  for (auto rechit = transient_track.recHitsBegin(); rechit != transient_track.recHitsEnd(); rechit++) {
    const DetId&& det_id = (*rechit)->geographicalId();
    if (MuonHitHelper::isGEM(det_id)) {
      continue;

    } else if (MuonHitHelper::isCSC(det_id)) {
      if ((*rechit)->dimension() != 4) {
        continue;
      }
    }

    const GeomDet* det = geometry->idToDet(det_id);
    const GlobalPoint&& global_position = det->toGlobal((*rechit)->localPosition());
    // const float distance = std::abs(layer.disk->localZclamped(global_position));
    const float z_distance = std::abs(layer.disk->localZ(global_position));
    if (z_distance < min_distance) {
      found = true;
      min_distance = z_distance;
      starting_point = global_position;
      start_id = det_id;
    }
  }

  TrajectoryStateOnSurface start_state;
  if (found) {
    start_state = transient_track.stateOnSurface(starting_point);
  }
  return std::make_pair(start_state, start_id);
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

bool GEMEfficiencyAnalyzer::checkBounds(const GlobalPoint& global_point,
                                        const GlobalError& global_error,
                                        const Plane& plane) {

  const LocalPoint&& local_point = plane.toLocal(global_point);
  const LocalError&& local_error = ErrorFrameTransformer::transform(global_error, plane);

  const LocalPoint local_point_2d{local_point.x(), local_point.y(), 0.0f};
  // https://github.com/cms-sw/cmssw/blob/CMSSW_12_0_0_pre3/DataFormats/GeometrySurface/src/SimpleDiskBounds.cc#L20-L35
  // TODO comment on the scale
  return plane.bounds().inside(local_point_2d, local_error, kBoundsErrorScale_);
}

const GEMEtaPartition* GEMEfficiencyAnalyzer::findEtaPartition(const GlobalPoint& global_point,
                                                               const GlobalError& global_error,
                                                               const std::vector<const GEMChamber*>& chamber_vector) {
  const GEMEtaPartition* bound = nullptr;
  for (const GEMChamber* chamber : chamber_vector) {
    // TODO
    if (not checkBounds(global_point, global_error, chamber->surface())) {
      continue;
    }

    for (const GEMEtaPartition* eta_partition : chamber->etaPartitions()) {
      if (checkBounds(global_point, eta_partition->surface())) {
        bound = eta_partition;
        break;
      }
    }  // GEMEtaPartition
  }    // GEMChamber

  return bound;
}


// Returns GEMRecHit
std::pair<const GEMRecHit*, float> GEMEfficiencyAnalyzer::findClosestHit(
    const GlobalPoint& prop_global_pos,
    const GEMEtaPartition* prop_eta_partition,
    const edm::Handle<GEMRecHitCollection>& rechit_collection,
    const edm::ESHandle<GEMGeometry>& gem) {

  const GEMRecHit* closest_hit = nullptr;
  float min_rdphi = 1e6f; // just big number

  const GEMDetId&& prop_gem_id = prop_eta_partition->id();
  const int prop_ieta = prop_gem_id.ieta();

  const GEMDetId&& chamber_id = prop_gem_id.chamberId();
  const GEMChamber* chamber = gem->chamber(chamber_id);
  if (chamber == nullptr) {
    edm::LogError(kLogCategory_) << chamber_id;
    return std::make_pair(closest_hit, min_rdphi);
  }

  //
  const LocalPoint&& prop_chamber_pos = chamber->toLocal(prop_global_pos);

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

      if (std::abs(rdphi) < std::abs(min_rdphi)) {
        closest_hit = &(*hit);
        min_rdphi = rdphi;
      }
    }
  }

  return std::make_pair(closest_hit, min_rdphi);
}
