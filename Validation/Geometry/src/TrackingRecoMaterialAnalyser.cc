#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/SiStripDetId/interface/TIBDetId.h"

#include "TrackingTools/TrackRefitter/interface/TrackTransformer.h"
#include "TrackingTools/PatternTools/interface/Trajectory.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"

#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESTransientHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDMException.h"

#include <assert.h>
#include <unordered_map>
#include <string>

// Values are not ordered randomly, but the order is taken from
// http://cmslxr.fnal.gov/dxr/CMSSW/source/Geometry/CommonDetUnit/interface/GeomDetEnumerators.h#15
static std::vector<std::string> sDETS{ "", "PXB", "PXF", "TIB", "TID", "TOB", "TEC" };
static std::vector<std::string> sLAYS{ "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

class TrackingRecoMaterialAnalyser : public DQMEDAnalyzer {
  public:
    explicit TrackingRecoMaterialAnalyser(const edm::ParameterSet&);
    virtual void bookHistograms(DQMStore::IBooker &i,
                                edm::Run const&,
                                edm::EventSetup const&) override;
    void analyze(const edm::Event &, const edm::EventSetup &) override ;
    virtual ~TrackingRecoMaterialAnalyser();
  private:
    bool isDoubleSided(DetId, const TrackerTopology &);
    TrackTransformer refitter_;
    const edm::EDGetTokenT<reco::TrackCollection>  tracksToken_;
    std::unordered_map<std::string, MonitorElement *> histosOriEta_;
    std::unordered_map<std::string, MonitorElement *> histosEta_;
    MonitorElement * histo_RZ_;
    MonitorElement * histo_RZ_Ori_;
};

//-------------------------------------------------------------------------
TrackingRecoMaterialAnalyser::TrackingRecoMaterialAnalyser(const edm::ParameterSet& iPSet):
  refitter_(iPSet),
  tracksToken_(consumes<reco::TrackCollection>(iPSet.getParameter<edm::InputTag>("tracks"))),
  histo_RZ_(0)
{
}

//-------------------------------------------------------------------------
TrackingRecoMaterialAnalyser::~TrackingRecoMaterialAnalyser(void)
{
}

//-------------------------------------------------------------------------
void TrackingRecoMaterialAnalyser::bookHistograms(DQMStore::IBooker & ibook,
                                                  edm::Run const&,
                                                  edm::EventSetup const &setup) {
  using namespace std;
  edm::ESHandle<TrackerGeometry> trackerGeometry;
  setup.get<TrackerDigiGeometryRecord>().get(trackerGeometry);

  ibook.setCurrentFolder("RecoMaterialFromRecoTracks");
  histo_RZ_Ori_ = ibook.bookProfile2D("OriRadLen", "Original_RadLen",
                                  600, -300., 300, 120, 0., 120., 0., 1.);
  histo_RZ_ = ibook.bookProfile2D("RadLen", "RadLen",
                                  600, -300., 300, 120, 0., 120., 0., 1.);
  char title[50];
  char key[20];
  for (unsigned int det = 1; det < sDETS.size(); ++det ) {
    for (unsigned int sub_det = 1;
      sub_det <= trackerGeometry->numberOfLayers(det); ++sub_det) {
      memset(title, 0, sizeof(title));
      snprintf(title, sizeof(title), "Original_RadLen_vs_Eta_%s%d", sDETS[det].data(), sub_det);
      snprintf(key, sizeof(key), "%s%d", sDETS[det].data(), sub_det);
      histosOriEta_.insert(make_pair<string, MonitorElement*>(key,
        ibook.bookProfile(title, title, 250, -5.0, 5.0, 0., 1.)));
      snprintf(title, sizeof(title), "RadLen_vs_Eta_%s%d", sDETS[det].data(), sub_det);
      histosEta_.insert(make_pair<string, MonitorElement*>(key,
        ibook.bookProfile(title, title, 250, -5.0, 5.0, 0., 1.)));
    }
  }
}

bool TrackingRecoMaterialAnalyser::isDoubleSided(DetId id, const TrackerTopology & trk_topology) {
  SiStripDetId strip_id(id);
  return (((strip_id.subDetector() == SiStripDetId::TIB) ||
           (strip_id.subDetector() == SiStripDetId::TOB) ||
           (strip_id.subDetector() == SiStripDetId::TID) ||
           (strip_id.subDetector() == SiStripDetId::TEC)) && strip_id.glued());
}

//-------------------------------------------------------------------------
void TrackingRecoMaterialAnalyser::analyze(const edm::Event& event,
                                           const edm::EventSetup& setup)
{
  using namespace edm;
  using namespace reco;
  using namespace std;

  refitter_.setServices(setup);

  Handle<TrackCollection> tracks;
  ESHandle<TrackerTopology> trk_topology;

  // Get the TrackerTopology
  setup.get<TrackerTopologyRcd>().get(trk_topology);

  event.getByToken(tracksToken_, tracks);
  if (!tracks.isValid() || tracks->size() == 0) {
    cout << "Invalid or empty track collection" << endl;
    return;
  }
  auto selector = [&](const Track &track) -> bool {
    return (track.pt() > 1.
            && track.quality(track.qualityByName("highPurity")));
  };

  // Main idea:
  // * select first good tracks in input, according to reasonable criteria
  // * refit the track so that we have access to the TrajectoryMeasurements
  //   that internally have all the information about the TSOS on all
  //   crossed layers. We need the refit track and not the original track so
  //   that we are able to correctly compute the path travelled by the track
  //   within the detector, using its updated TSOS. The material description
  //   can in principle be derived also directly from the rechits, via the
  //   det()[(->)GeomDet *]->mediumProperties chain, but that would simply give the
  //   face values, not the "real" ones used while propagating the track.
  // * Loop on all measurements, extract the information about the TSOS,
  //   its surface and its mediumProperties
  // * Make plots for the untouched material properties, but also for the
  //   ones corrected by the track direction, since the material properties,
  //   according to the documentation, should refer to normal incidence of
  //   the track, which is seldom the case, according to the current direction
  TrajectoryStateOnSurface current_tsos;
  DetId current_det;
  for (auto const track : *tracks) {
    if (!selector(track)  and false)
      continue;
    vector<Trajectory> traj  = refitter_.transform(track);
    if (traj.size() > 1 || traj.size() == 0)
      continue;
    for (auto const &tm : traj.front().measurements()) {
      if (tm.recHit().get() &&
        (tm.recHitR().type() == TrackingRecHit::valid ||
         tm.recHitR().type() == TrackingRecHit::missing)) {
        current_tsos = tm.updatedState().isValid() ? tm.updatedState() : tm.forwardPredictedState();
        auto const & localP = current_tsos.localMomentum();
        current_det = tm.recHit()->geographicalId();
        const Surface& surface = current_tsos.surface();
        assert(tm.recHit()->surface() == &surface);
        if (!surface.mediumProperties().isValid()) {
          LogError("TrackingRecoMaterialAnalyser")
            << "Medium properties for material linked to detector "
            << " are invalid!" << endl;
          continue;
        }
        float p2 = localP.mag2();
        float xf = std::abs(std::sqrt(p2)/localP.z());
//        float e2     = p2 + m2;
//        float beta2  = p2/e2;
        float ori_xi = surface.mediumProperties().xi();
        float ori_radLen = surface.mediumProperties().radLen();
        float xi = ori_xi*xf;
        float radLen = ori_radLen*xf;

        // Since there are double-sided (glued) modules all over the tracker,
        // the material budget has been internally partitioned in two equal
        // components, so that on each single layer will receive half of the
        // correct radLen. For this reason, only for the double-sided
        // components, we rescale the obtained radLen by 2.
        // In particular see code here: http://cmslxr.fnal.gov/dxr/CMSSW_8_0_5/source/Geometry/TrackerGeometryBuilder/src/TrackerGeomBuilderFromGeometricDet.cc#213
        // where, in the SiStrip Tracker, if the module has a partner
        // (i.e. it's a glued detector) the plane is built with a scaling of
        // 0.5. The actual plane is built few lines below:
        // http://cmslxr.fnal.gov/dxr/CMSSW_8_0_5/source/Geometry/TrackerGeometryBuilder/src/TrackerGeomBuilderFromGeometricDet.cc#287

        if (isDoubleSided(current_det, *trk_topology)) {
          LogTrace("TrackingRecoMaterialAnalyser") <<  "Eta: " << track.eta() << " "
             << sDETS[current_det.subdetId()]+sLAYS[trk_topology->layer(current_det)]
             << " has ori_radLen: " << ori_radLen << " and ori_xi: " << xi
             << " and has radLen: " << radLen << "  and xi: " << xi << endl;
          ori_radLen *= 2.;
          radLen *= 2.;
        }

        histosOriEta_[sDETS[current_det.subdetId()]+sLAYS[trk_topology->layer(current_det)]]->Fill(current_tsos.globalPosition().eta(), ori_radLen);
        histosEta_[sDETS[current_det.subdetId()]+sLAYS[trk_topology->layer(current_det)]]->Fill(current_tsos.globalPosition().eta(), radLen);
        histo_RZ_Ori_->Fill(current_tsos.globalPosition().z(), current_tsos.globalPosition().perp(), ori_radLen);
        histo_RZ_->Fill(current_tsos.globalPosition().z(), current_tsos.globalPosition().perp(), radLen);
        LogInfo("TrackingRecoMaterialAnalyser") <<  "Eta: " << track.eta() << " "
             << sDETS[current_det.subdetId()]+sLAYS[trk_topology->layer(current_det)]
             << " has ori_radLen: " << ori_radLen << " and ori_xi: " << xi
             << " and has radLen: " << radLen << "  and xi: " << xi << endl;
      }
    }
  }
}

//-------------------------------------------------------------------------
// define as a plugin
#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(TrackingRecoMaterialAnalyser);
