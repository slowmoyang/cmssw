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
      kMuonTrackTypeName_(ps.getUntrackedParameter<std::string>("muonTrackType")),
      kMuonTrackType_(getMuonTrackType(ps.getUntrackedParameter<std::string>("muonTrackType"))),
      kName_(ps.getUntrackedParameter<std::string>("name")),
      kFolder_(ps.getUntrackedParameter<std::string>("folder")),
      kIsCosmics_(ps.getUntrackedParameter<bool>("isCosmics")),
      kMuonPtMinCut_(static_cast<float>(ps.getUntrackedParameter<double>("muonPtMinCut"))),
      kMuonAbsEtaMinCut_(static_cast<float>(ps.getUntrackedParameter<double>("muonAbsEtaMinCut"))),
      kMuonAbsEtaMaxCut_(static_cast<float>(ps.getUntrackedParameter<double>("muonAbsEtaMaxCut"))),
      kAllowedCSCs(getAllowedCSCs(ps)),
      kUseSkipLayer_(ps.getUntrackedParameter<bool>("useSkipLayer")),
      kPropagationRErrorCut_(static_cast<float>(ps.getUntrackedParameter<double>("propagationRErrorCut"))),
      kPropagationPhiErrorCut_(static_cast<float>(ps.getUntrackedParameter<double>("propagationPhiErrorCut"))),
      kBoundsErrorScale_(static_cast<float>(ps.getUntrackedParameter<double>("boundsErrorScale"))),
      kRdPhiCut_(static_cast<float>(ps.getUntrackedParameter<double>("rdphiCut"))),
      kPtBins_(ps.getUntrackedParameter<std::vector<double> >("ptBins")),
      kEtaNBins_(ps.getUntrackedParameter<int>("etaNbins")),
      kEtaLow_(ps.getUntrackedParameter<double>("etaLow")),
      kEtaUp_(ps.getUntrackedParameter<double>("etaUp")),
      kMonitorGE11_(ps.getUntrackedParameter<bool>("monitorGE11")),
      kMonitorGE21_(ps.getUntrackedParameter<bool>("monitorGE21")),
      kMonitorGE0_(ps.getUntrackedParameter<bool>("monitorGE0")) {

  muon_service_ = std::make_unique<MuonServiceProxy>(
      ps.getParameter<edm::ParameterSet>("ServiceParameters"), consumesCollector());
}

GEMEfficiencyAnalyzer::~GEMEfficiencyAnalyzer() {}

void GEMEfficiencyAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("recHitTag", edm::InputTag("gemRecHits"));
  desc.add<edm::InputTag>("muonTag", edm::InputTag("muons"));
  desc.addUntracked<std::string>("muonTrackType", "CombinedTrack");
  desc.addUntracked<std::string>("name", "Global Muon");
  desc.addUntracked<std::string>("folder", "GEM/Efficiency/Muon");
  desc.addUntracked<bool>("isCosmics", false);
  desc.addUntracked<double>("muonPtMinCut", 20.0f);
  desc.addUntracked<double>("muonAbsEtaMinCut", 1.4);
  desc.addUntracked<double>("muonAbsEtaMaxCut", 2.3);
  desc.addUntracked<std::vector<int> >("allowedCSCsForGE11", {1, 2});
  desc.addUntracked<std::vector<int> >("allowedCSCsForGE21", {});
  desc.addUntracked<std::vector<int> >("allowedCSCsForGE0", {});
  desc.addUntracked<bool>("useSkipLayer", true);
  desc.addUntracked<double>("propagationRErrorCut", 0.5); // cm
  desc.addUntracked<double>("propagationPhiErrorCut", 0.1); // degree
  desc.addUntracked<double>("boundsErrorScale", -2);
  desc.addUntracked<double>("rdphiCut", 2.0);  // TODO need to be tuned
  desc.addUntracked<std::vector<double> >("ptBins", {0., 5., 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., 120.});
  desc.addUntracked<int>("etaNbins", 20);
  desc.addUntracked<double>("etaLow", 1.0);
  desc.addUntracked<double>("etaUp", 3.0);
  desc.addUntracked<bool>("monitorGE11", true);
  desc.addUntracked<bool>("monitorGE21", false);
  desc.addUntracked<bool>("monitorGE0", false);

  edm::ParameterSetDescription service_parameters;
  service_parameters.setAllowAnything();
  desc.add<edm::ParameterSetDescription>("ServiceParameters", service_parameters);

  descriptions.add("gemEfficiencyAnalyzerDefault", desc);
}

void GEMEfficiencyAnalyzer::bookHistograms(DQMStore::IBooker& ibooker,
                                           edm::Run const& run,
                                           edm::EventSetup const& setup) {
  const edm::ESHandle<GEMGeometry> gem = setup.getHandle(kGEMTokenBeginRun_);
  if (not gem.isValid()) {
    edm::LogError(kLogCategory_) << "invalid GEMGeometry. failed to bookHistograms";
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
  auto hist = dynamic_cast<TH1F*>(me->getTH1F()->Clone(name.c_str()));
  return ibooker.book1D(name, hist);
}

dqm::impl::MonitorElement* GEMEfficiencyAnalyzer::bookNumerator2D(DQMStore::IBooker& ibooker, MonitorElement* me) {
  const std::string name = me->getName() + "_matched";
  auto hist = dynamic_cast<TH2F*>(me->getTH2F()->Clone(name.c_str()));
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

    if (skipGEMStation(station_id)) {
      continue;
    }

    const GEMDetId key = getReStKey(region_id, station_id);
    const TString name_suffix = GEMUtils::getSuffixName(region_id, station_id);
    const TString title_suffix = GEMUtils::getSuffixTitle(region_id, station_id);

    const TString title = kName_.c_str() + title_suffix;

    // TODO check if kPtBins_ is empty
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

    if (skipGEMStation(station_id)) {
      continue;
    }

    const std::vector<const GEMSuperChamber*> superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs";
      return;
    }

    const int num_chambers = superchambers.size();
    for (const GEMChamber* chamber : superchambers.front()->chambers()) {
      const int layer_id = chamber->id().layer();

      const TString name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
      const TString title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
      const GEMDetId key = getReStLaKey(chamber->id());

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

    if (skipGEMStation(station_id)) {
      continue;
    }

    const std::vector<const GEMSuperChamber*> superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs";
      return;
    }

    for (const GEMChamber* chamber : superchambers.front()->chambers()) {
      const int layer_id = chamber->id().layer();
      const int num_ieta = chamber->nEtaPartitions();

      const TString name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
      const TString title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
      const GEMDetId key = getReStLaKey(chamber->id());

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

    if (skipGEMStation(station_id)) {
      continue;
    }

    const std::vector<const GEMSuperChamber*> superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs";
      return;
    }

    const int num_ch = superchambers.size();

    for (const GEMChamber* chamber : superchambers.front()->chambers()) {
      const int layer_id = chamber->id().layer();
      const int num_ieta = chamber->nEtaPartitions();

      const TString name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
      const TString title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);
      const GEMDetId key = getReStLaKey(chamber->id());

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

    if (skipGEMStation(station_id)) {
      continue;
    }

    const std::vector<const GEMSuperChamber*> superchambers = station->superChambers();
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

      const GEMDetId key = getReStEtKey(eta_partition->id());
      // TODO
      const TString name_suffix = TString::Format("_GE%d1-%s-E%d", station_id, (region_id > 0 ? "P" : "M"), ieta);
      const TString title =
          kName_.c_str() + TString::Format(" GE%d1-%s-E%d", station_id, (region_id > 0 ? "P" : "M"), ieta);

      me_rdphi_[key] =
          ibooker.book1D("rdphi" + name_suffix, title, 50, -kRdPhiCut_, kRdPhiCut_);
      me_rdphi_[key]->setAxisTitle("R#Delta#phi [cm]");

    }  // ieta
  }    // station
}

void GEMEfficiencyAnalyzer::bookMisc(DQMStore::IBooker& ibooker, const edm::ESHandle<GEMGeometry>& gem) {
  ibooker.setCurrentFolder(kFolder_ + "/Misc");

  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();

    if (skipGEMStation(station_id)) {
      continue;
    }

    const std::vector<const GEMSuperChamber*> superchambers = station->superChambers();
    if (not checkRefs(superchambers)) {
      edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs";
      return;
    }
    // ignore layer ids
    const int num_ch = superchambers.size();

    const GEMDetId key = getReStKey(region_id, station_id);
    const TString name_suffix = GEMUtils::getSuffixName(region_id, station_id);
    const TString title_suffix = GEMUtils::getSuffixTitle(region_id, station_id);

    me_all_abs_rdphi_[key] = ibooker.book1D("all_abs_rdphi", ";R#Delta#phi [cm];Entries", 20, 0.0, 2*kRdPhiCut_);

    //
    me_prop_chamber_[key] = ibooker.book1D("prop_chamber" + name_suffix, title_suffix, num_ch, 0.5, num_ch + 0.5);
    me_prop_chamber_[key]->setAxisTitle("Destination Chamber Id", 1);
    me_prop_chamber_[key]->setAxisTitle("Entries", 2);

    if (kMuonTrackType_ == reco::Muon::MuonTrackType::InnerTrack) {
      // TRK muon
      // See the links below for more details:
      // https://github.com/cms-sw/cmssw/blob/CMSSW_12_3_0_pre4/DataFormats/SiPixelDetId/interface/PixelSubdetector.h
      // https://github.com/cms-sw/cmssw/blob/CMSSW_12_3_0_pre4/DataFormats/SiStripDetId/interface/SiStripEnums.h
      me_start_det_[key] = ibooker.book1D("start_det" + name_suffix, title_suffix, 6, 0.5, 6.5);
      const std::vector<std::string> tracker_subdetector_names = {
        "PixelBarrel",
        "PixelEndcap",
        "TIB",
        "TID",
        "TOB",
        "TEC"
      };
      for (size_t idx = 0; idx < tracker_subdetector_names.size(); idx++) {
        const int bin = static_cast<int>(idx) + 1;
        const std::string label = tracker_subdetector_names.at(idx);
        me_start_det_[key]->setBinLabel(bin, label, 1);
      }

    } else {
      // STA or GLB muon
      // See the links below for more details:
      // https://github.com/cms-sw/cmssw/blob/CMSSW_12_3_0_pre4/DataFormats/MuonDetId/interface/CSCDetId.h#L187-L193
      // https://github.com/cms-sw/cmssw/blob/CMSSW_12_3_0_pre4/DataFormats/MuonDetId/interface/CSCDetId.h#L250
      me_start_det_[key] = ibooker.book1D("start_det" + name_suffix, title_suffix, 10, 0.5, 10.5);
      for (int chamber_type = 1; chamber_type <= 10; chamber_type++) {
        const std::string label = CSCDetId::chamberName(chamber_type);
        me_start_det_[key]->setBinLabel(chamber_type, label, 1);
      }
    }
    me_start_det_matched_[key] = bookNumerator1D(ibooker, me_start_det_[key]);

    me_prop_err_r_[key] = ibooker.book1D("prop_err_r" + name_suffix, title_suffix, 200, 0.0, 2.0);
    me_prop_err_r_matched_[key] = bookNumerator1D(ibooker, me_prop_err_r_[key]);

    me_prop_err_phi_[key] = ibooker.book1D("prop_err_phi" + name_suffix, title_suffix, 100, 0.0, 1.0);
    me_prop_err_phi_matched_[key] = bookNumerator1D(ibooker, me_prop_err_phi_[key]);

  }
}

reco::Muon::MuonTrackType GEMEfficiencyAnalyzer::getMuonTrackType(const std::string& name) {
  reco::Muon::MuonTrackType muon_track_type;

  // NOTE do NOT allow typo
  if (name == "InnerTrack") {
    muon_track_type = reco::Muon::MuonTrackType::InnerTrack;

  } else if (name == "OuterTrack") {
    muon_track_type = reco::Muon::MuonTrackType::OuterTrack;

  } else if (name == "CombinedTrack") {
    muon_track_type = reco::Muon::MuonTrackType::CombinedTrack;

  } else {
    muon_track_type = reco::Muon::MuonTrackType::None;
    edm::LogError(kLogCategory_) << "got a wrong MuonTrackType label: " << name << ". All events will be skipped.";

  }

  return muon_track_type;
}

std::map<int, std::vector<int> > GEMEfficiencyAnalyzer::getAllowedCSCs(const edm::ParameterSet& ps) {
  // GEM station to allowed CSC components
  std::map<int, std::vector<int> > output = {
    {1, ps.getUntrackedParameter<std::vector<int> >("allowedCSCsForGE11")}, 
    {2, ps.getUntrackedParameter<std::vector<int> >("allowedCSCsForGE21")}, 
    {0, ps.getUntrackedParameter<std::vector<int> >("allowedCSCsForGE0")}, 
  };

  return output;
}

bool GEMEfficiencyAnalyzer::skipGEMStation(const int station) {
  bool skip = false;

  if (station == 1) {
    if (not kMonitorGE11_) {
      LogDebug(kLogCategory_) << "skip GE11 because monitorGE11 is " << std::boolalpha << kMonitorGE11_;
      skip = true;
    }

  } else if (station == 2) {
    if (not kMonitorGE21_) {
      LogDebug(kLogCategory_) << "skip GE21 because monitorGE21 is " << std::boolalpha << kMonitorGE21_;
      skip = true;
    }

  } else if (station == 0) {
    if (not kMonitorGE0_) {
      LogDebug(kLogCategory_) << "skip GE0 because monitorGE0 is " << std::boolalpha << kMonitorGE0_;
      skip = true;
    }

  } else {
    edm::LogError(kLogCategory_) << "got an unexpected GEM station " << station << ". skip this station.";
    skip = true;
  }

  return skip;
}

// In the case of GE11, the odd and even chambers are located on different
// planes, so they are constructed in different layers.
std::vector<GEMEfficiencyAnalyzer::GEMLayerData> GEMEfficiencyAnalyzer::buildGEMLayers(
    const edm::ESHandle<GEMGeometry>& gem) {
  std::vector<GEMLayerData> layer_vector;

  for (const GEMStation* station : gem->stations()) {
    const int region_id = station->region();
    const int station_id = station->station();
    const bool is_ge11 = station_id == 1;

    if (skipGEMStation(station_id)) {
      continue;
    }

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
      const Disk::DiskPointer layer = Disk::build(position, rotation, bounds);

      layer_vector.emplace_back(layer, chamber_vector, region_id, station_id, layer_id);
    }  // layer
  }    // GEMStation

  return layer_vector;
}

// Returns a starting state of the propagation and corresponding detector id.
// 1. In the case of InnerTrack, this method simply use the outermost measurement state
// as the starting state.
// If the muon is a standalone muon or a global muon, this method finds a (TODO) CSC segment.
std::pair<TrajectoryStateOnSurface, DetId> GEMEfficiencyAnalyzer::buildStartingState(
    const reco::Muon& muon,
    const reco::TransientTrack& transient_track,
    const GEMLayerData& layer,
    const edm::ESHandle<GlobalTrackingGeometry>& global_tracking_geometry) {

  TrajectoryStateOnSurface start_state;
  DetId start_id;

  if (kMuonTrackType_ == reco::Muon::MuonTrackType::InnerTrack) {
    std::tie(start_state, start_id) = findOutermostMeasurementState(transient_track);

  } else {
    // TODO how about using GE0 segments for GE11?

    // standalone muon or global muon
    const std::vector<const CSCSegment*> candidates = kIsCosmics_ ? findCSCSegments(muon, layer.station) : findCSCSegments(transient_track, layer.station);

    // TODO 
    if (const CSCSegment* csc_segment = findCSCSegmentWithMinZDistance(candidates, layer, global_tracking_geometry)) {
      std::tie(start_state, start_id) = buildStartingStateFromCSCSegment(csc_segment, global_tracking_geometry, transient_track);
    }

  }

  return std::make_pair(start_state, start_id);
}


// If we use the inner track, we simply use the outermost measurement as the
// starting state. However, the order of `TrackingRecHit`s may be reversed in
// the cosmics scenario.
// FIXME is it true for the inner track (?)
std::pair<TrajectoryStateOnSurface, DetId> GEMEfficiencyAnalyzer::findOutermostMeasurementState(const reco::TransientTrack& transient_track) {

  const reco::Track& track = transient_track.track();
  const bool is_insideout = isTrackInsideOut(track);
  const DetId start_id = is_insideout ? track.innerDetId() : track.outerDetId();
  const TrajectoryStateOnSurface start_state = is_insideout ? transient_track.innermostMeasurementState() : transient_track.outermostMeasurementState();
  return std::make_pair(start_state, start_id);
}

// Returns a bool value indicating whether or not the CSC detector can be used as a start detector for a given GEM station.
bool GEMEfficiencyAnalyzer::isCSCAllowed(const CSCDetId& csc_id, const int gem_station) {
  const int csc_chamber_type = static_cast<int>(csc_id.iChamberType());
  const std::vector<int>& allowed = kAllowedCSCs.at(gem_station);
  return allowed.empty() or (std::find(allowed.begin(), allowed.end(), csc_chamber_type) != allowed.end());
}

std::vector<const CSCSegment*> GEMEfficiencyAnalyzer::findCSCSegments(
    const reco::TransientTrack& transient_track,
    const int gem_station) {

  std::vector<const CSCSegment*> csc_segment_vector;
  for (trackingRecHit_iterator tracking_rechit_iter = transient_track.recHitsBegin(); tracking_rechit_iter != transient_track.recHitsEnd(); tracking_rechit_iter++) {
    const TrackingRecHit* tracking_rechit = (*tracking_rechit_iter);
    if (not tracking_rechit->isValid()) {
      edm::LogError(kLogCategory_) << "got an invalid TrackingRecHit from reco::TransientTrack."
                                   << "TrackingRecHit::type = " << static_cast<int>(tracking_rechit->type());
      continue;
    }


    const DetId det_id = tracking_rechit->geographicalId();

    if (not MuonHitHelper::isCSC(det_id)) {
      continue;
    }

    if (tracking_rechit->dimension() != kCSCSegmentDimension_) {
      continue;
    }

    const CSCDetId csc_id{det_id};
    if (not isCSCAllowed(csc_id, gem_station)) {
      continue;
    }

    if (auto csc_segment = dynamic_cast<const CSCSegment*>(tracking_rechit)) {
      csc_segment_vector.push_back(csc_segment);

    } else {
      edm::LogError(kLogCategory_) << "failed to perform the conversion from `const TrackingRechit*` to `const CSCSegment*`";

    }
  } // trackingRecHit_iterator

  return csc_segment_vector;
}

// Returns a vector of CSCSegments matching the reconstructed muon track.
// In the `cosmics` scenario, the hits in the segment are used for the track
// reconstruction instead of the segment itself. Therefore, it is required to
// find CSCSegments matching with track.
// See https://doi.org/10.1140/epjc/s10052-008-0674-7 for more details
std::vector<const CSCSegment*> GEMEfficiencyAnalyzer::findCSCSegments(const reco::Muon& muon,
                                                                      const int gem_station) {
  std::vector<const CSCSegment*> segment_vector;

  for (const reco::MuonChamberMatch& chamber_match : muon.matches()) {
    if (chamber_match.detector() != MuonSubdetId::CSC) {
      continue;
    }

    for (const reco::MuonSegmentMatch& segment_match : chamber_match.segmentMatches) {
      const CSCSegment* csc_segment = segment_match.cscSegmentRef.get();

      if (not isCSCAllowed(csc_segment->cscDetId(), gem_station)) {
        continue;
      }

      segment_vector.push_back(csc_segment);

    }
  }

  return segment_vector;
}

const CSCSegment* GEMEfficiencyAnalyzer::findCSCSegmentWithMinZDistance(
    std::vector<const CSCSegment*> csc_segment_vector,
    const GEMLayerData& layer,
    const edm::ESHandle<GlobalTrackingGeometry>& global_tracking_geometry) {

  const CSCSegment* output = nullptr;
  float min_z_distance = std::numeric_limits<float>::infinity();

  for (const CSCSegment* segment : csc_segment_vector) {
    const DetId det_id{segment->rawId()};
    const GeomDet* det = global_tracking_geometry->idToDet(det_id);
    const GlobalPoint global_position = det->toGlobal(segment->localPosition());
    const float z_distance = std::abs(layer.disk->localZclamped(global_position));
    if (z_distance < min_z_distance) {
      min_z_distance = z_distance;
      output = segment;
    }
  } // const CSCSegment*

  return output;
}

std::pair<TrajectoryStateOnSurface, DetId> GEMEfficiencyAnalyzer::buildStartingStateFromCSCSegment(
    const CSCSegment* csc_segment,
    const edm::ESHandle<GlobalTrackingGeometry>& global_tracking_geometry,
    const reco::TransientTrack& transient_track) {

  const DetId det_id{csc_segment->rawId()};
  const GeomDet* det = global_tracking_geometry->idToDet(det_id);
  const GlobalPoint& global_position = det->toGlobal(csc_segment->localPosition());
  const TrajectoryStateOnSurface state = transient_track.stateOnSurface(global_position);
  return std::make_pair(state, det_id);
}

// This method true if the propagation direction is invalid.
bool GEMEfficiencyAnalyzer::skipPropagation(const reco::Track* track, const GEMLayerData& layer) {
  if (not kUseSkipLayer_) {
    return false;
  }

  const bool is_same_region = track->eta() * layer.region > 0;

  bool skip = false;
  if (kIsCosmics_) {
    float p2_in = track->innerMomentum().mag2();
    float p2_out = track->outerMomentum().mag2();
    if (isTrackInsideOut(*track))
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
  const LocalPoint local_point = plane.toLocal(global_point);
  const LocalPoint local_point_2d(local_point.x(), local_point.y(), 0.0f);
  return plane.bounds().inside(local_point_2d);
}

bool GEMEfficiencyAnalyzer::checkBounds(const GlobalPoint& global_point,
                                        const GlobalError& global_error,
                                        const Plane& plane) {

  const LocalPoint local_point = plane.toLocal(global_point);
  const LocalError local_error = ErrorFrameTransformer::transform(global_error, plane);

  const LocalPoint local_point_2d{local_point.x(), local_point.y(), 0.0f};
  // https://github.com/cms-sw/cmssw/blob/CMSSW_12_0_0_pre3/DataFormats/GeometrySurface/src/SimpleDiskBounds.cc#L20-L35
  // TODO comment on the scale
  return plane.bounds().inside(local_point_2d, local_error, kBoundsErrorScale_);
}

// Find the GEMEtaPartition where the propagated muon is located. When checking
// whether the muon is in the chamber, an error in the position of the
// propagated muon is considered.
const GEMEtaPartition* GEMEfficiencyAnalyzer::findEtaPartition(const GlobalPoint& global_point,
                                                               const GlobalError& global_error,
                                                               const std::vector<const GEMChamber*>& chamber_vector) {
  const GEMEtaPartition* bound = nullptr;
  for (const GEMChamber* chamber : chamber_vector) {
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

std::pair<const GEMRecHit*, float> GEMEfficiencyAnalyzer::findClosestHit(
    const GlobalPoint& prop_global_pos,
    const GEMEtaPartition* prop_eta_partition,
    const edm::Handle<GEMRecHitCollection>& rechit_collection,
    const edm::ESHandle<GEMGeometry>& gem) {

  const GEMRecHit* closest_hit = nullptr;
  float min_rdphi = std::numeric_limits<float>::infinity();

  const GEMDetId prop_gem_id = prop_eta_partition->id();
  const int prop_ieta = prop_gem_id.ieta();

  const GEMDetId chamber_id = prop_gem_id.chamberId();
  const GEMChamber* chamber = gem->chamber(chamber_id);
  if (chamber == nullptr) {
    edm::LogError(kLogCategory_) << chamber_id;
    return std::make_pair(closest_hit, min_rdphi);
  }

  //
  const LocalPoint prop_chamber_pos = chamber->toLocal(prop_global_pos);

  for (const GEMEtaPartition* hit_eta_partition : chamber->etaPartitions()) {
    const GEMDetId hit_gem_id = hit_eta_partition->id();
    const int delta_ieta = std::abs(prop_ieta - hit_gem_id.ieta());
    if (delta_ieta > 1) {
      continue;
    }

    const GEMRecHitCollection::range& rechit_range = rechit_collection->get(hit_gem_id);
    for (auto hit = rechit_range.first; hit != rechit_range.second; ++hit) {
      const GlobalPoint hit_global_pos = hit_eta_partition->toGlobal(hit->localPosition());
      const LocalPoint hit_chamber_pos = chamber->toLocal(hit_global_pos);

      const float hit_strip = hit_eta_partition->strip(hit->localPosition());
      const float hit_strip_angle = hit_eta_partition->specificTopology().stripAngle(hit_strip);

      const float dx = prop_chamber_pos.x() - hit_chamber_pos.x();
      const float dy = prop_chamber_pos.y() - hit_chamber_pos.y();

      const float rdphi = std::cos(hit_strip_angle) * dx - std::sin(hit_strip_angle) * dy;

      if (std::abs(rdphi) < std::abs(min_rdphi)) {
        closest_hit = &(*hit);
        min_rdphi = rdphi;
      }
    } // GEMRecHit
  } // GEMEtaPartition

  return std::make_pair(closest_hit, min_rdphi);
}

int GEMEfficiencyAnalyzer::getStartDetBin(const DetId& start_id) {
  int bin = 0;

  const DetId::Detector detector = start_id.det();
  const int subdet = start_id.subdetId();

  if ((kMuonTrackType_ == reco::Muon::MuonTrackType::InnerTrack)) {
    if (detector == DetId::Detector::Tracker) {
      bin = subdet;

    }

  } else {
    if (MuonHitHelper::isCSC(start_id.rawId())) {
      const CSCDetId csc_id{start_id};
      bin = csc_id.iChamberType();     

    }

  }

  if (bin == 0) {
    edm::LogError(kLogCategory_) << "MuonTrackType is " << kMuonTrackTypeName_
                                 << " but got an unexpected starting detector: "
                                 << "DetId::detector=" << static_cast<int>(detector)
                                 << ", DetId::subdetId=" << subdet;
  }

  return bin;
}

void GEMEfficiencyAnalyzer::analyze(const edm::Event& event, const edm::EventSetup& setup) {
  if UNLIKELY (kMuonTrackType_ == reco::Muon::MuonTrackType::None) {
    return;
  }

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
  //
  const std::vector<GEMLayerData> layer_vector = buildGEMLayers(gem);
  // TODO unlikely
  if UNLIKELY (layer_vector.empty()) {
    edm::LogError(kLogCategory_) << "failed to build GEM layers"; 
    return;
  }

  //////////////////////////////////////////////////////////////////////////////
  for (const reco::Muon& muon : *muon_view) {
    const reco::Track* track = muon.muonTrack(kMuonTrackType_).get();
    if (track == nullptr) {
      LogDebug(kLogCategory_) << "failed to get a reco::Track from the reco::Muon";
      continue;
    }

    const reco::TransientTrack transient_track = transient_track_builder->build(track);
    if (not transient_track.isValid()) {
      edm::LogError(kLogCategory_) << "failed to build TransientTrack";
      continue;
    }

    for (const GEMLayerData& layer : layer_vector) {
      if (skipPropagation(track, layer)) {
        // TODO print layer id and the direction of muon.
        LogDebug(kLogCategory_) << "skip GEM Layer";
        continue;
      }

      const auto& [start_state, start_id] = buildStartingState(muon, transient_track, layer, global_tracking_geometry);
      if (not start_state.isValid()) {
        LogDebug(kLogCategory_) << "failed to get a starting state";
        continue;
      }

      const TrajectoryStateOnSurface propagated_state = propagator->propagate(start_state, *layer.disk);
      if (not propagated_state.isValid()) {
        LogDebug(kLogCategory_) << "failed to propagate a muon";
        continue;
      }

      const GlobalPoint prop_global_pos = propagated_state.globalPosition();
      const GlobalError& prop_global_err = ErrorFrameTransformer::transform(
          propagated_state.localError().positionError(), *layer.disk);

      if (not checkBounds(prop_global_pos, prop_global_err, *layer.disk)) {
        LogDebug(kLogCategory_) << "failed to pass checkBounds";
        continue;
      }

      const int start_det_bin = getStartDetBin(start_id);
      const double prop_global_r_err = std::sqrt(prop_global_err.rerr(prop_global_pos));
      const double prop_global_phi_err = std::sqrt(prop_global_err.phierr(prop_global_pos));
      const double prop_global_phi_err_deg = TMath::RadToDeg() * prop_global_phi_err;
      const bool passed_prop_err_cut = (prop_global_r_err < kPropagationRErrorCut_) and (prop_global_phi_err_deg < kPropagationPhiErrorCut_);

      const GEMEtaPartition* eta_partition = findEtaPartition(prop_global_pos, prop_global_err, layer.chambers);
      if (eta_partition == nullptr) {
        LogDebug(kLogCategory_) << "failed to find an eta partition";
        continue;
      }

      const GEMDetId gem_id = eta_partition->id();
      const GEMDetId rs_key = getReStKey(gem_id);
      const GEMDetId rsl_key = getReStLaKey(gem_id);
      const GEMDetId rse_key = getReStEtKey(gem_id);
      const int chamber_id = gem_id.chamber();
      const int ieta = gem_id.ieta();
      const double muon_pt = muon.pt();
      const double muon_eta = std::fabs(muon.eta());

      const bool passed_pt_cut = muon_pt > kMuonPtMinCut_;
      const bool passed_eta_cut = (muon_eta > kMuonAbsEtaMinCut_) and (muon_eta < kMuonAbsEtaMaxCut_);
      const bool passed_all_cuts = passed_prop_err_cut and passed_pt_cut and passed_eta_cut;

      fillME(me_prop_chamber_, rs_key, chamber_id);

      fillME(me_start_det_, rs_key, start_det_bin);
      fillMEWithinLimits(me_prop_err_r_, rs_key, prop_global_r_err);
      fillMEWithinLimits(me_prop_err_phi_, rs_key, prop_global_phi_err_deg);

      if (passed_prop_err_cut and passed_eta_cut) {
        fillMEWithinLimits(me_muon_pt_, rs_key, muon_pt);
      }

      if (passed_prop_err_cut and passed_pt_cut) {
        fillMEWithinLimits(me_muon_eta_, rs_key, muon_eta);
      }

      if (passed_all_cuts) {
        fillME(me_muon_phi_, rs_key, muon.phi());
        fillME(me_chamber_, rsl_key, chamber_id);
        fillME(me_ieta_, rsl_key, ieta);
        fillME(me_detector_, rsl_key, chamber_id, ieta);
      }

      //////////////////////////////////////////////////////////////////////////
      //
      const auto& [hit, rdphi] = findClosestHit(start_state.globalPosition(),
                                                eta_partition,
                                                rechit_collection,
                                                gem);
      if (hit == nullptr) {
        LogDebug(kLogCategory_) << "failed to find a hit";
        continue;
      }

      fillMEWithinLimits(me_all_abs_rdphi_, rs_key, std::abs(rdphi));
      if (std::abs(rdphi) > kRdPhiCut_) {
        LogDebug(kLogCategory_) << "failed to pass the rdphi cut";
        continue;
      }

      //////////////////////////////////////////////////////////////////////////
      // MEs for 
      fillME(me_start_det_matched_, rs_key, start_det_bin);
      fillMEWithinLimits(me_prop_err_r_matched_, rs_key, prop_global_r_err);
      fillMEWithinLimits(me_prop_err_phi_matched_, rs_key, prop_global_phi_err_deg);

      if (passed_prop_err_cut and passed_eta_cut) {
        fillMEWithinLimits(me_muon_pt_matched_, rs_key, muon_pt);
      }

      if (passed_prop_err_cut and passed_pt_cut) {
        fillMEWithinLimits(me_muon_eta_matched_, rs_key, muon_eta);
      }

      if (passed_all_cuts) {
        fillME(me_muon_phi_matched_, rs_key, muon.phi());
        fillME(me_chamber_matched_, rsl_key, gem_id.chamber());
        fillME(me_ieta_matched_, rsl_key, gem_id.ieta());
        fillME(me_detector_matched_, rsl_key, gem_id.chamber(), ieta);
      }

      //////////////////////////////////////////////////////////////////////////
      // Fill resolutions
      if (passed_all_cuts) {
        fillMEWithinLimits(me_rdphi_, rse_key, rdphi);
      }
    }  // layer
  }    // Muon
}
