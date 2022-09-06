#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/GEMRecHit/interface/GEMCSCSegmentCollection.h"
#include "DataFormats/GEMDigi/interface/GEMVFATStatusCollection.h"
#include "DataFormats/GEMDigi/interface/GEMOHStatusCollection.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "DataFormats/MuonReco/interface/MuonChamberMatch.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "TTree.h"

class GEMCSCCoincidenceRateAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit GEMCSCCoincidenceRateAnalyzer(const edm::ParameterSet&);
  ~GEMCSCCoincidenceRateAnalyzer();
  static void fillDescriptions(edm::ConfigurationDescriptions &);

private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&);

  void resetBranches();

  bool analyzeGEMCSCSegment(const GEMCSCSegment&,
                            const GEMOHStatusCollection*,
                            const GEMVFATStatusCollection*,
                            const std::vector<const CSCSegment*>);


  bool checkGEMChamberStatus(const GEMDetId& chamber_id, const GEMOHStatusCollection*, const GEMVFATStatusCollection*);
  std::vector<const CSCSegment*> findMuonSegments(const reco::MuonCollection *);
  bool isMuonSegment(const CSCSegment &, const std::vector<const CSCSegment*>);


  static constexpr int kGEMNumLayers_ = 2;

  const edm::EDGetTokenT<GEMCSCSegmentCollection> gem_csc_segment_collection_token_;
  const edm::EDGetTokenT<GEMOHStatusCollection> gem_oh_status_collection_token_;
  const edm::EDGetTokenT<GEMVFATStatusCollection> gem_vfat_status_collection_token_;
  const edm::EDGetTokenT<reco::MuonCollection> muon_collection_token_;
	const std::string log_category_;
  const bool use_gem_daq_status_;


  //
  TTree* tree_;
  int b_region_;
  int b_station_;
  int b_ring_;
  int b_chamber_;
  bool b_gem_chamber_has_error_;
  // GEMCSCSegment
  float b_norm_chi2_;
  // CSCSegment
  float b_csc_norm_chi2_;
  int b_csc_num_hit_;
  bool b_csc_is_muon_;
  // GEMRecHit
  int b_gem_num_hit;
  bool b_gem_found_[kGEMNumLayers_];
  int b_gem_layer_[kGEMNumLayers_];
  int b_gem_ieta_[kGEMNumLayers_];
  int b_gem_first_strip_[kGEMNumLayers_];
  int b_gem_cls_[kGEMNumLayers_];
};

GEMCSCCoincidenceRateAnalyzer::GEMCSCCoincidenceRateAnalyzer(const edm::ParameterSet& ps)
    : gem_csc_segment_collection_token_(
          consumes<GEMCSCSegmentCollection>(ps.getUntrackedParameter<edm::InputTag>("gemcscSegmentTag"))),
      gem_oh_status_collection_token_(
          consumes<GEMOHStatusCollection>(ps.getUntrackedParameter<edm::InputTag>("ohStatusTag"))),
      gem_vfat_status_collection_token_(
          consumes<GEMVFATStatusCollection>(ps.getUntrackedParameter<edm::InputTag>("vfatStatusTag"))),
      muon_collection_token_(consumes<reco::MuonCollection>(ps.getUntrackedParameter<edm::InputTag>("muonTag"))),
		  log_category_(ps.getUntrackedParameter<std::string>("logCategory")),
      use_gem_daq_status_(ps.getUntrackedParameter<bool>("useGEMDAQStatus")) {

  usesResource(TFileService::kSharedResource);

  edm::Service<TFileService> file_service;
  tree_ = file_service->make<TTree>("gemcsc", "gemcsc");

  tree_->Branch("region", &b_region_);
  tree_->Branch("station", &b_station_);
  tree_->Branch("ring", &b_ring_);
  tree_->Branch("chamber", &b_chamber_);
  tree_->Branch("gem_chamber_has_error", &b_gem_chamber_has_error_, "gem_chamber_has_error/O");

  tree_->Branch("norm_chi2", &b_norm_chi2_, "norm_chi2/F");
  tree_->Branch("csc_norm_chi2", &b_csc_norm_chi2_, "csc_norm_chi2/F");
  tree_->Branch("csc_num_hit", &b_csc_num_hit_, "csc_num_hit/I");
  tree_->Branch("csc_is_muon", &b_csc_is_muon_, "csc_is_muon/O");

  tree_->Branch("gem_num_hit", &b_gem_num_hit, "gem_num_hit/I");
  tree_->Branch("gem_found", &b_gem_found_, "gem_found[2]/O");
  tree_->Branch("gem_layer", &b_gem_layer_, "gem_layer[2]/I");
  tree_->Branch("gem_ieta", &b_gem_ieta_, "gem_ieta[2]/I");
  tree_->Branch("gem_first_strip", &b_gem_first_strip_, "gem_first_strip[2]/I");
  tree_->Branch("gem_cls", &b_gem_cls_, "gem_cls[2]/I");
  tree_->Branch("gem_found", &b_gem_found_, "gem_found[2]/I");

}

GEMCSCCoincidenceRateAnalyzer::~GEMCSCCoincidenceRateAnalyzer() {
}

void GEMCSCCoincidenceRateAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.addUntracked<edm::InputTag>("ohStatusTag", edm::InputTag("muonGEMDigis", "OHStatus"));
  desc.addUntracked<edm::InputTag>("vfatStatusTag", edm::InputTag("muonGEMDigis", "VFATStatus"));
  desc.addUntracked<std::string>("logCategory", "GEMCSCCoincidenceRateAnalyzer");
  desc.addUntracked<edm::InputTag>("gemcscSegmentTag", edm::InputTag("gemcscSegments"));
  desc.addUntracked<edm::InputTag>("muonTag", edm::InputTag("muons"));
  desc.addUntracked<bool>("useGEMDAQStatus", false);
  descriptions.addWithDefaultLabel(desc);
}


void GEMCSCCoincidenceRateAnalyzer::analyze(const edm::Event& event, const edm::EventSetup&) {
  ///////////////////////////////////////////////////////////////////////////// 
	// get data from edm::Event
  ///////////////////////////////////////////////////////////////////////////// 
  const GEMCSCSegmentCollection* gem_csc_segment_collection = nullptr;
  if (const edm::Handle<GEMCSCSegmentCollection> handle = event.getHandle(gem_csc_segment_collection_token_)) {
    gem_csc_segment_collection = handle.product();

  } else {
    edm::LogError(log_category_) << "failed to get GEMCSCSegmentCollection";
    return;
  }

  const GEMOHStatusCollection* gem_oh_status_collection = nullptr;
  const GEMVFATStatusCollection* gem_vfat_status_collection = nullptr;
  if (use_gem_daq_status_) {
    if (auto handle = event.getHandle(gem_oh_status_collection_token_)) {
      gem_oh_status_collection = handle.product();

    } else {
      edm::LogError(log_category_) << "failed to get OHVFATStatusCollection";
      return;
    }

    if (auto handle = event.getHandle(gem_vfat_status_collection_token_)) {
      gem_vfat_status_collection = handle.product();

    } else {
      edm::LogError(log_category_) << "failed to get GEMVFATStatusCollection";
      return;
    }
  }

  const reco::MuonCollection* muon_collection = nullptr;
	if (const edm::Handle<reco::MuonCollection> handle = event.getHandle(muon_collection_token_)) {
		muon_collection = handle.product();

	} else {
		edm::LogError(log_category_) << "failed to get reco::MuonCollection";
		return;
	}

  ///////////////////////////////////////////////////////////////////////////// 
  // analyze
  ///////////////////////////////////////////////////////////////////////////// 
  const std::vector<const CSCSegment*> muon_segment_vec = findMuonSegments(muon_collection);

  for (edm::OwnVector<GEMCSCSegment>::const_iterator iter = gem_csc_segment_collection->begin();
       iter != gem_csc_segment_collection->end();
       iter++) {
    const GEMCSCSegment& gem_csc_segment = *iter;

    const CSCDetId csc_id = gem_csc_segment.cscDetId();
    if (csc_id.isME11() or csc_id.isME21()) {
      resetBranches();
      const bool okay = analyzeGEMCSCSegment(gem_csc_segment, gem_oh_status_collection, gem_vfat_status_collection, muon_segment_vec);
      if (okay) {
        tree_->Fill();
      }
    }
  }  // GEMCSCSegment
}


void GEMCSCCoincidenceRateAnalyzer::resetBranches() {
	// detector
  b_region_ = 0;
  b_station_ = 0;
  b_ring_ = 0;
  b_chamber_ = 0;
  b_gem_chamber_has_error_ = false;
  // GEMCSCSegment
  b_norm_chi2_ = 0.0f;
  // CSCSegment
  b_csc_norm_chi2_ = 0.0f;
  b_csc_num_hit_ = 0;
  b_csc_is_muon_ = false;
  // GEMRecHit
  b_gem_num_hit = 0;
  for (size_t gem_hit_idx = 0; gem_hit_idx < kGEMNumLayers_; ++gem_hit_idx) {
		b_gem_found_[gem_hit_idx] = false;
		b_gem_layer_[gem_hit_idx] = 0;
		b_gem_ieta_[gem_hit_idx] = 0;
		b_gem_first_strip_[gem_hit_idx] = 0;
		b_gem_cls_[gem_hit_idx] = 0;
	} // GEMRecHit
}



bool GEMCSCCoincidenceRateAnalyzer::analyzeGEMCSCSegment(const GEMCSCSegment& gem_csc_segment,
                                                         const GEMOHStatusCollection* gem_oh_status_collection,
                                                         const GEMVFATStatusCollection* gem_vfat_status_collection,
                                                         const std::vector<const CSCSegment*> muon_segment_vec) {
  const CSCDetId csc_id = gem_csc_segment.cscDetId();
  const CSCSegment csc_segment = gem_csc_segment.cscSegment();
  const std::vector<GEMRecHit> gem_hit_vec = gem_csc_segment.gemRecHits();
  // GEMDetId(int region, int ring, int station, int layer, int chamber, int ieta)
	const GEMDetId gem_chamber_id{csc_id.zendcap(), 1, csc_id.station(), 0, csc_id.chamber(), 0};

	// detector
  b_region_ = csc_id.zendcap();
  b_station_ = csc_id.station();
  b_ring_ = csc_id.ring();
  b_chamber_ = csc_id.chamber();
  b_gem_chamber_has_error_ = use_gem_daq_status_ ? checkGEMChamberStatus(gem_chamber_id, gem_oh_status_collection, gem_vfat_status_collection) : false;
  // GEMCSCSegment
  b_norm_chi2_ = static_cast<float>(gem_csc_segment.chi2() / gem_csc_segment.nRecHits());
  // CSCSegment
  b_csc_norm_chi2_ = static_cast<float>(csc_segment.chi2() / csc_segment.nRecHits());
  b_csc_num_hit_ = csc_segment.nRecHits();
  b_csc_is_muon_ = isMuonSegment(csc_segment, muon_segment_vec);
  // GEMRecHit
  b_gem_num_hit = gem_csc_segment.gemRecHits().size();
	for (const GEMRecHit& gem_hit : gem_csc_segment.gemRecHits()) {
		const GEMDetId gem_id = gem_hit.gemId();
		const int gem_layer = gem_id.layer();
		const size_t gem_hit_idx = gem_layer - 1;
		if (gem_hit_idx > 1) {
			edm::LogError(log_category_) << "wrong gem id: " << gem_id;
			continue;
		}

		b_gem_found_[gem_hit_idx] = true;
		b_gem_layer_[gem_hit_idx] = gem_layer;
		b_gem_ieta_[gem_hit_idx] = gem_id.ieta();
		b_gem_first_strip_[gem_hit_idx] = gem_hit.firstClusterStrip();
		b_gem_cls_[gem_hit_idx] = gem_hit.clusterSize();
	} // GEMRecHit

  return true;
}


std::vector<const CSCSegment*> GEMCSCCoincidenceRateAnalyzer::findMuonSegments(const reco::MuonCollection* muon_collection) {
  std::vector<const CSCSegment*> muon_segment_vec;

  for (const reco::Muon& muon : *muon_collection) { 
    for (const reco::MuonChamberMatch& chamber_match : muon.matches()) {
      if (chamber_match.detector() != MuonSubdetId::CSC) {
        continue;
      }

      const CSCDetId csc_id{chamber_match.id};
      if (csc_id.isME11() or csc_id.isME21()) {
        for (const reco::MuonSegmentMatch& segment_match : chamber_match.segmentMatches) {
          if (segment_match.isMask(reco::MuonSegmentMatch::BestInStationByDR)) {
            muon_segment_vec.push_back(segment_match.cscSegmentRef.get());
            break;
          }
        }  // MuonSegmentMatch
      } // isME11 or isME21
    }    // MuonChamberMatch
  }      // MuonCollection
  
  return muon_segment_vec;
}

// Returns a boolean indicating whether 
bool GEMCSCCoincidenceRateAnalyzer::isMuonSegment(const CSCSegment& csc_segment,
                                                         const std::vector<const CSCSegment*> muon_segment_vec) {
  bool found = false;

  const CSCDetId csc_id = csc_segment.cscDetId();
  for (const CSCSegment* muon_segment : muon_segment_vec) {
    if (csc_id != muon_segment->cscDetId())
      continue;
    if (csc_segment.localPosition().x() != muon_segment->localPosition().x())
      continue;
    if (csc_segment.localPosition().y() != muon_segment->localPosition().y())
      continue;
    if (csc_segment.localPosition().z() != muon_segment->localPosition().z())
      continue;
    if (csc_segment.time() != muon_segment->time())
      continue;

    found = true;
  }

  return found;
}

bool GEMCSCCoincidenceRateAnalyzer::checkGEMChamberStatus(const GEMDetId& chamber_id,
                                                      const GEMOHStatusCollection* oh_status_collection,
                                                      const GEMVFATStatusCollection* vfat_status_collection) {
  const bool has_error = true;
  if (not use_gem_daq_status_) {
    edm::LogError(log_category_) << "useGEMDAQStatus is false but checkGEMChamberStatus is called. gem_chamber_has_error will be set to false";
    return not has_error;
  }

  for (auto iter = oh_status_collection->begin(); iter != oh_status_collection->end(); iter++) {
    const auto [oh_id, range] = (*iter);
    if (chamber_id != oh_id) {
      continue;
    }

    for (auto oh_status = range.first; oh_status != range.second; oh_status++) {
      if (oh_status->isBad()) {
        // GEMOHStatus is bad. Mask this chamber.
        return has_error;
      }  // isBad
    }    // range
  }      // collection

  for (auto iter = vfat_status_collection->begin(); iter != vfat_status_collection->end(); iter++) {
    const auto [vfat_id, range] = (*iter);
    if (chamber_id != vfat_id.chamberId()) {
      continue;
    }
    for (auto vfat_status = range.first; vfat_status != range.second; vfat_status++) {
      if (vfat_status->isBad()) {
        return has_error;
      }
    }  // range
  }    // collection

  return not has_error;
}

DEFINE_FWK_MODULE(GEMCSCCoincidenceRateAnalyzer);
