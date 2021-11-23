#ifndef DQMOffline_Muon_GE11EffME11SegmentAnalyzer_h
#define DQMOffline_Muon_GE11EffME11SegmentAnalyzer_h

/** \class GE11EffME11SegmentAnalyzer
 * 
 *
 */

#include "DQMOffline/Muon/interface/GEMOfflineDQMBase.h"
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
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"


class GE11EffME11SegmentAnalyzer : public GEMOfflineDQMBase {
public:
  explicit GE11EffME11SegmentAnalyzer(const edm::ParameterSet &);
  ~GE11EffME11SegmentAnalyzer() override;
  static void fillDescriptions(edm::ConfigurationDescriptions &);

protected:
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
  void analyze(const edm::Event &event, const edm::EventSetup &eventSetup) override;

private:
  // main
  void propagate(const reco::Muon&,
                 const edm::ESHandle<TransientTrackBuilder>&,
                 const edm::ESHandle<Propagator>&,
                 const edm::Handle<GEMRecHitCollection>&,
                 const edm::ESHandle<GEMGeometry>&,
                 const edm::ESHandle<CSCGeometry>&);
  const CSCSegment* findME11Segment(const reco::Muon&);
  TrajectoryStateOnSurface getStartingState(const CSCSegment*,
                                            const reco::Muon&,
                                            const edm::ESHandle<CSCGeometry>&);

  bool checkBounds(const GlobalPoint&, const Plane&);
  bool checkBounds(const GlobalPoint &, const GlobalError&, const Plane &);
  bool checkFiducialCut(const GlobalPoint&, const GEMEtaPartition*);
  bool isGoodPropagation(const TrajectoryStateOnSurface&,
                         const GEMEtaPartition*,
                         const TrajectoryStateOnSurface&);

  std::pair<const GEMRecHit*, float> findClosestHit(
      const GlobalPoint&,
      const GEMEtaPartition*,
      const edm::Handle<GEMRecHitCollection>&,
      const edm::ESHandle<GEMGeometry>&);



  // helper
  MonitorElement *bookNumerator1D(DQMStore::IBooker &, MonitorElement *);
  MonitorElement *bookNumerator2D(DQMStore::IBooker &, MonitorElement *);
  reco::Muon::MuonTrackType getMuonTrackType(const std::string&);
  bool isME11(const DetId&);

  // MonitorElement
  MEMap me_chamber_;  // 2D, region-station-layer
  MEMap me_chamber_matched_;

  MonitorElement * me_me11_strip_region_;
  MonitorElement * me_me11_strip_region_matched_;

  MonitorElement* me_has_inner_muon_;
  MonitorElement* me_has_inner_muon_matched_;

  // parameters
  const edm::ESGetToken<GEMGeometry, MuonGeometryRecord>                      kGEMToken_;
  const edm::ESGetToken<GEMGeometry, MuonGeometryRecord>                      kGEMTokenBeginRun_;
  const edm::ESGetToken<CSCGeometry, MuonGeometryRecord>                      kCSCToken_;
  const edm::ESGetToken<GlobalTrackingGeometry, GlobalTrackingGeometryRecord> kGlobalTrackingGeometryToken_;
  const edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord>          kTrasientTrackToken_;

  // parmaeters from ParameterSet
  const edm::EDGetTokenT<GEMRecHitCollection>                                 kRecHitToken_;
  const edm::EDGetTokenT<edm::View<reco::Muon> >                              kMuonToken_;
  const reco::Muon::MuonTrackType kMuonTrackType_;
  const bool kIsCosmics_;
  const bool kUseME11A_;
  const bool kUseFiducialCut_;
  const float kFiducialCutY_; // cm
  const float kFiducialCutPhi_; // degree
  const float kMinRdPhi_; // cm * radian
  const std::string kFolder_;


  std::unique_ptr<MuonServiceProxy> muon_service_;





  // const
  const std::string kLogCategory_ = "GE11EffME11SegmentAnalyzer";
};

#endif  // DQMOffline_Muon_GE11EffME11SegmentAnalyzer_h
