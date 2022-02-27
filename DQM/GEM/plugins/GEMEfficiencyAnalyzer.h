#ifndef DQM_GEM_GEMEfficiencyAnalyzer_h
#define DQM_GEM_GEMEfficiencyAnalyzer_h

/** \class GEMEfficiencyAnalyzer
 * 
 * DQM monitoring source for GEM efficiency and resolution
 * based on https://github.com/CPLUOS/MuonPerformance/blob/master/MuonAnalyser/plugins/SliceTestEfficiencyAnalysis.cc
 *
 * \author Seungjin Yang <seungjin.yang@cern.ch>
 */

#include "DQM/GEM/interface/GEMOfflineDQMBase.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "Geometry/CommonTopologies/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"


class GEMEfficiencyAnalyzer : public GEMOfflineDQMBase {
public:
  explicit GEMEfficiencyAnalyzer(const edm::ParameterSet &);
  ~GEMEfficiencyAnalyzer() override;
  static void fillDescriptions(edm::ConfigurationDescriptions &);

protected:
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
  void analyze(const edm::Event &event, const edm::EventSetup &eventSetup) override;

private:
  struct GEMLayerData {
    GEMLayerData(Disk::DiskPointer disk, std::vector<const GEMChamber *> chambers, int region, int station, int layer)
        : disk(disk), chambers(chambers), region(region), station(station), layer(layer) {}
    Disk::DiskPointer disk;
    std::vector<const GEMChamber *> chambers;
    int region, station, layer;
  };

  MonitorElement *bookNumerator1D(DQMStore::IBooker &, MonitorElement *);
  MonitorElement *bookNumerator2D(DQMStore::IBooker &, MonitorElement *);

  void bookEfficiencyMomentum(DQMStore::IBooker &, const edm::ESHandle<GEMGeometry> &);
  void bookEfficiencyChamber(DQMStore::IBooker &, const edm::ESHandle<GEMGeometry> &);
  void bookEfficiencyEtaPartition(DQMStore::IBooker &, const edm::ESHandle<GEMGeometry> &);
  void bookEfficiencyDetector(DQMStore::IBooker &, const edm::ESHandle<GEMGeometry> &);
  void bookResolution(DQMStore::IBooker &, const edm::ESHandle<GEMGeometry> &);
  void bookMisc(DQMStore::IBooker &, const edm::ESHandle<GEMGeometry> &);

  inline bool isTrackInsideOut(const reco::Track &);

  bool skipGEMStation(const int);
  reco::Muon::MuonTrackType getMuonTrackType(const std::string&);
  std::map<int, std::vector<int> > getAllowedCSCs(const edm::ParameterSet&);

  std::vector<GEMLayerData> buildGEMLayers(const edm::ESHandle<GEMGeometry> &);

  std::pair<TrajectoryStateOnSurface, DetId> buildStartingState(const reco::Muon&,
                                                                const reco::TransientTrack &,
                                                                const GEMLayerData &,
                                                                const edm::ESHandle<GlobalTrackingGeometry> &);

  std::pair<TrajectoryStateOnSurface, DetId> findOutermostMeasurementState(const reco::TransientTrack&);

  std::vector<const CSCSegment*> findCSCSegments(const reco::TransientTrack&, const int);
  std::vector<const CSCSegment*> findCSCSegments(const reco::Muon&, const int);
  bool isCSCAllowed(const CSCDetId&, const int);
  const CSCSegment* findCSCSegmentWithMinZDistance(std::vector<const CSCSegment*>,
                                                   const GEMLayerData&,
                                                   const edm::ESHandle<GlobalTrackingGeometry>&);

  std::pair<TrajectoryStateOnSurface, DetId> buildStartingStateFromCSCSegment(
      const CSCSegment*,
      const edm::ESHandle<GlobalTrackingGeometry>&,
      const reco::TransientTrack&);



  bool isME11(const DetId &);
  bool skipPropagation(const reco::Track *, const GEMLayerData &);
  bool checkBounds(const GlobalPoint &, const Plane &);
  bool checkBounds(const GlobalPoint &, const GlobalError&, const Plane &);
  const GEMEtaPartition *findEtaPartition(const GlobalPoint &, const GlobalError&, const std::vector<const GEMChamber *> &);
  std::pair<const GEMRecHit*, float> findClosestHit(
      const GlobalPoint&,
      const GEMEtaPartition*,
      const edm::Handle<GEMRecHitCollection>&,
      const edm::ESHandle<GEMGeometry>&);

  int getStartDetBin(const DetId&);

  // data members

  // parameters
  const edm::ESGetToken<GEMGeometry, MuonGeometryRecord> kGEMToken_;
  const edm::ESGetToken<GEMGeometry, MuonGeometryRecord> kGEMTokenBeginRun_;
  const edm::ESGetToken<GlobalTrackingGeometry, GlobalTrackingGeometryRecord> kGlobalTrackingGeometryToken_;
  const edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> kTrasientTrackToken_;
  const edm::EDGetTokenT<GEMRecHitCollection> kRecHitToken_;
  const edm::EDGetTokenT<edm::View<reco::Muon> > kMuonToken_;
  const std::string kMuonTrackTypeName_;
  const reco::Muon::MuonTrackType kMuonTrackType_;
  const std::string kName_;
  const std::string kFolder_;
  const bool kIsCosmics_;
  const float kMuonPtMinCut_;
  const float kMuonAbsEtaMinCut_;
  const float kMuonAbsEtaMaxCut_;
  const std::map<int, std::vector<int> > kAllowedCSCs; // `GEMDetId::station` to a vector `CSCDetId::iChamberType`s
  const bool kUseSkipLayer_;
  const float kPropagationRErrorCut_; // cm
  const float kPropagationPhiErrorCut_; // degree
  const float kBoundsErrorScale_;
  const float kRdPhiCut_; // cm * radian
  const std::vector<double> kPtBins_;
  const int kEtaNBins_;
  const double kEtaLow_;
  const double kEtaUp_;
  bool kMonitorGE11_;
  bool kMonitorGE21_;
  bool kMonitorGE0_;

  std::unique_ptr<MuonServiceProxy> muon_service_;

  // MonitorElement
  // efficiency
  MEMap me_muon_pt_;  // 1D, region-station
  MEMap me_muon_pt_matched_;
  MEMap me_muon_eta_;  // 1D, region-station
  MEMap me_muon_eta_matched_;
  MEMap me_muon_phi_;  // 1D, region-station
  MEMap me_muon_phi_matched_;
  MEMap me_chamber_;  // 2D, region-station-layer
  MEMap me_chamber_matched_;
  MEMap me_ieta_;  // 1D, region-station-layer
  MEMap me_ieta_matched_;
  MEMap me_detector_;  // 2D, region-station-layer
  MEMap me_detector_matched_;
  // resolution
  MEMap me_rdphi_;  // global
  // MEs for optimizing cut values
  MEMap me_all_abs_rdphi_;
  MEMap me_prop_chamber_;
  MEMap me_start_det_;
  MEMap me_start_det_matched_;
  MEMap me_prop_err_r_;
  MEMap me_prop_err_r_matched_;
  MEMap me_prop_err_phi_;
  MEMap me_prop_err_phi_matched_;

  // const
  const std::string kLogCategory_ = "GEMEfficiencyAnalyzer";
  // https://github.com/cms-sw/cmssw/blob/CMSSW_12_3_0_pre4/DataFormats/CSCRecHit/interface/CSCSegment.h#L60
  const int kCSCSegmentDimension_ = 4;
};

// FIXME
inline bool GEMEfficiencyAnalyzer::isTrackInsideOut(const reco::Track &track) {
  return track.innerPosition().mag2() > track.outerPosition().mag2();
}

#endif  // DQM_GEM_GEMEfficiencyAnalyzer_h
