#include "Validation/MuonGEMRecHits/interface/GEMRecHitsValidation.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"


GEMRecHitsValidation::GEMRecHitsValidation(const edm::ParameterSet& ps)
    : GEMBaseValidation(ps) {
  auto sim_hit_label = ps.getParameter<edm::InputTag>("simInputLabel");
  auto rec_hit_label = ps.getParameter<edm::InputTag>("recHitsInputLabel");

  sim_hit_token_ = consumes<edm::PSimHitContainer>(sim_hit_label);
  rec_hit_token_ = consumes<GEMRecHitCollection>(rec_hit_label);

  detailPlot_ = ps.getParameter<Bool_t>("detailPlot");
  folder_ = ps.getParameter<std::string>("folder");
}


GEMRecHitsValidation::~GEMRecHitsValidation() {
}


void GEMRecHitsValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                          edm::Run const & Run,
                                          edm::EventSetup const & iSetup) {

  const GEMGeometry* kGEM = initGeometry(iSetup);
  if ( kGEM == nullptr) {
    edm::LogError(kLogCategory_) << "Failed to initialise GEMGeometry in the "
                                 << "bookHistograms step." << std::endl;
    return ;  
  }

  ibooker.setCurrentFolder(folder_);
  // TODO subdirectory

  // Book histograms
  for(const auto & region : kGEM->regions()) {
    Int_t region_id = region->region();

    if(auto tmp_me = bookZROccupancy(ibooker, region_id, "rechit", "RecHit")) {
      me_occ_zr_[region_id] = tmp_me;
    } else {
      edm::LogError(kLogCategory_) << "Failed to create simpleZR histograms\n";
    }

    for(const auto & station : region->stations()) {
      Int_t station_id = station->station();
      ME2IdsKey key2(region_id, station_id);

      // FIXME 
      me_sim_occ_det_[key2] = bookDetectorOccupancy(ibooker, key2, station, "simhit", "SimHit");
      me_rec_occ_det_[key2] = bookDetectorOccupancy(ibooker, key2, station, "rechit", "RecHit");
    } // station loop end

    me_pull_x_[region_id] = bookHist1D(ibooker, region_id, "pull_x", "Pull Of X", 100, -5, 5);
    me_pull_y_[region_id] = bookHist1D(ibooker, region_id, "pull_y", "Pull Of Y", 100, -5, 5);

  } // region loop end

  me_cls_ = ibooker.book1D("cls_tot", "Cluster Size Distribution", 11, -0.5, 10.5);
 
  // TODO logging 
  const Double_t kPi = TMath::Pi();

  if(detailPlot_) {
    for(const auto & region : kGEM->regions()) {
      Int_t region_id = region->region();

      for(const auto & station : region->stations()) {
        Int_t station_id = station->station();

        // TODO
        for(Int_t layer_id : {1, 2}) {

          ME3IdsKey key(region_id, station_id, layer_id);

          me_detail_occ_zr_[key] = bookZROccupancy(
              ibooker, key, "rechit", "RecHits");
          me_detail_occ_xy_[key] = bookXYOccupancy(
              ibooker, key, "rechit", "RecHits");
          me_detail_occ_xy_ch1_[key] = bookXYOccupancy(
              ibooker, key, "rechit", "RecHits", "ch1", "(Chamber 1 Only)");
          me_detail_occ_polar_[key] = bookPolarOccupancy(
              ibooker, key, "rechit", "recHits");

          // bookHist1D(ibooker, name, title, nbinsx, xlow, xup, region_id, station_id, layer_id)
          me_detail_cls_[key] = bookHist1D(
              ibooker, key, "cls", "Cluster Size Distribution", 0, 10, "# of fired strips");

          // Occupancy histograms of SimHits and RecHits for Efficiency
          me_detail_residual_x_[key] = bookHist1D(
              ibooker, key, "residual_x", "Residual Of X", 100, -5, 5);

          me_detail_residual_y_[key] = bookHist1D(
              ibooker, key, "residual_y", "Residual Of Y", 100, -5, 5);

          me_detail_pull_x_[key] = bookHist1D(
              ibooker, key, "pull_x", "Pull Of X", 100, -5, 5);

          me_detail_pull_y_[key] = bookHist1D(
              ibooker, key, "pull_y", "Pull Of Y", 100, -5, 5);

          me_detail_sim_occ_eta_[key] = bookHist1D(
              ibooker, key, "simhit_occ_eta", "SimHit Eta Occupancy",
              51, -4, 4, "#eta");

          me_detail_rec_occ_eta_[key] = bookHist1D(
              ibooker, key, "rechit_occ_eta", "RecHit Eta Occupancy",
              51, -4, 4, "#eta");

          me_detail_sim_occ_phi_[key] = bookHist1D(
              ibooker, key, "simhit_occ_phi", "SimHit Phi Occupancy",
              51, -kPi, kPi, "#phi");

          me_detail_rec_occ_phi_[key] = bookHist1D(
              ibooker, key, "rechit_occ_phi", "RecHit Phi Occupancy",
              51, -kPi, kPi, "#phi");

          me_debug_unmatched_strip_diff_[key] = bookHist1D(
              ibooker, key, "debug_unmatched_strip_diff",
              "SimHit-Rechit Unmatched Case Strip Distance",
              -10, 10, "# of strips");

          me_debug_unmatched_cls_strip_diff_[key] = bookHist2D(
              ibooker, key, "debug_unmatched_cls_strip_diff",
              "SimHit-Rechit Unmatched Case cls vs. Strip Distance",
              -10, 10, // [xlow, xup]
              0, 10, // [ylow, yup]
              "# of strips", "cls");


        } // end loop over layers
      } // station loop
    } // region loop

  } // detailPlot 

}



void GEMRecHitsValidation::analyze(const edm::Event& e,
                                   const edm::EventSetup& iSetup) {

  const GEMGeometry* kGEM = initGeometry(iSetup);
  if (kGEM == nullptr) {
    edm::LogError(kLogCategory_) << "Failed to init GEMGeometry.\n";
    return; 
  }

  edm::Handle<edm::PSimHitContainer> simhit_container;
  e.getByToken(sim_hit_token_, simhit_container);
  if (not simhit_container.isValid()) {
    edm::LogError(kLogCategory_) << "Failed to get PSimHitContainer.\n";
    return ;
  }

  edm::Handle<GEMRecHitCollection> rechit_collection;
  e.getByToken(rec_hit_token_, rechit_collection);
  if (not rechit_collection.isValid()) {
    edm::LogError(kLogCategory_) << "Failed to get GEMRecHitCollection\n";
    return ;
  }

  // TODO if not isMC rechit occupancy only

  for(const auto & simhit : *simhit_container.product()) {

    if (std::abs(simhit.particleType()) != kMuonPDGId_) {
      edm::LogInfo(kLogCategory_) << "PSimHit is not muon.\n";
      continue;
    }

    if (kGEM->idToDet(simhit.detUnitId()) == nullptr) {
      // FIXME should I replace LogError as LogWarning or LogError?
      // NOTE
      edm::LogInfo(kLogCategory_) << "simHit did not matched with GEMGeometry.\n";
      continue;
    }

    const UInt_t kSimDetUnitId = simhit.detUnitId();
    GEMDetId gem_id(kSimDetUnitId);

    // FIXME change the name
    const Int_t region_id = gem_id.region();
    const Int_t station_id = gem_id.station();
    const Int_t layer_id = gem_id.layer();
    const Int_t chamber_id = gem_id.chamber();
    const Int_t roll_id = gem_id.roll();

    ME2IdsKey key2(region_id, station_id);
    ME3IdsKey key3(region_id, station_id, layer_id);

    // FIXME find good names
    LocalPoint sim_local = simhit.localPosition();
    GlobalPoint sim_global = kGEM->idToDet(gem_id)->surface().toGlobal(sim_local);

    Float_t simhit_global_eta = sim_global.eta();
    Float_t simhit_global_phi = sim_global.phi();

    // NOTE
    // GEMGeometry.h
    // returns fractional strip number [0..nstrips] for a LocalPoint
    // E.g., if local point hit strip #2, the fractional strip number would be
    // somewhere in the (1., 2] interval
    const Int_t kSimHitStrip = static_cast<Int_t>(std::ceil(kGEM->etaPartition(kSimDetUnitId)->strip(sim_local)));

    if(detailPlot_) {
      me_detail_sim_occ_eta_[key3]->Fill(simhit_global_eta);
      me_detail_sim_occ_phi_[key3]->Fill(simhit_global_phi);
    }

    const Int_t kDetOccBinX = getDetOccBinX(chamber_id, layer_id);
    me_sim_occ_det_[key2]->Fill(kDetOccBinX, roll_id);

    GEMRecHitCollection::range range = rechit_collection->get(gem_id);
    // GEMRecHitCollection::const_iterator;
    for(auto rechit = range.first; rechit != range.second; ++rechit) {
      Int_t cls = rechit->clusterSize();
      ///////////////////////////////
      // NOTE Start matching conditions
      ///////////////////////////
      Bool_t matched;
      if ( cls == 1 ) {
        matched = kSimHitStrip == rechit->firstClusterStrip();

        if(not matched) {
          Int_t strip_diff = rechit->firstClusterStrip() - kSimHitStrip;
          me_debug_unmatched_strip_diff_[key3]->Fill(strip_diff);
          me_debug_unmatched_cls_strip_diff_[key3]->Fill(strip_diff, cls);
        }

      } else {
        // fired strips
        const Int_t rec_first_strip = rechit->firstClusterStrip();
        const Int_t rec_last_strip = rec_first_strip + cls - 1;

        matched = (kSimHitStrip >= rec_first_strip) and (kSimHitStrip <= rec_last_strip);

        if(not matched) {
          Int_t first_diff = rec_first_strip - kSimHitStrip;
          Int_t last_diff = rec_last_strip - kSimHitStrip;

          Int_t strip_diff = std::abs(first_diff) < std::abs(last_diff) ? first_diff : last_diff;

          me_debug_unmatched_strip_diff_[key3]->Fill(strip_diff);
          me_debug_unmatched_cls_strip_diff_[key3]->Fill(strip_diff, cls);
        }
      }
      ////////////////////////////////////////////
      // NOTE End Matching
      ///////////////////////////////////////////

      if(matched) {
        LocalPoint rec_local = rechit->localPosition();
        GlobalPoint rec_global = kGEM->idToDet(kSimDetUnitId)->surface().toGlobal(rec_local);

        // RecHit Global 
        Float_t rechit_global_x = rec_global.x();
        Float_t rechit_global_y = rec_global.y();
        Float_t rechit_global_abs_z = std::fabs(rec_global.z());
        Float_t rechit_global_r = rec_global.perp();
        Float_t rechit_global_phi = rec_global.phi();

        Float_t  resolution_x = std::sqrt(rechit->localPositionError().xx());
        Float_t  resolution_y = std::sqrt(rechit->localPositionError().yy());

        Float_t residual_x = rec_local.x() - sim_local.x();
        Float_t residual_y = rec_local.y() - sim_local.y(); 

        Float_t pull_x = residual_x / resolution_x;
        Float_t pull_y = residual_y / resolution_y;

        /////////////////////////////////////////////
        //
        ///////////////////////////////////////////

        me_occ_zr_[region_id]->Fill(rechit_global_abs_z, rechit_global_r);

        me_rec_occ_det_[key2]->Fill(kDetOccBinX, roll_id);

        me_cls_->Fill(cls);
        me_pull_x_[region_id]->Fill(pull_x);
        me_pull_y_[region_id]->Fill(pull_y);

        if(detailPlot_) {
          me_detail_cls_[key3]->Fill(cls);

          me_detail_residual_x_[key3]->Fill(residual_x);
          me_detail_residual_y_[key3]->Fill(residual_y);

          me_detail_pull_x_[key3]->Fill(pull_x);
          me_detail_pull_y_[key3]->Fill(pull_y);

          me_detail_occ_zr_[key3]->Fill(rechit_global_abs_z, rechit_global_r);
          me_detail_occ_xy_[key3]->Fill(rechit_global_x, rechit_global_y);
          me_detail_occ_polar_[key3]->Fill(rechit_global_phi, rechit_global_r);

          // NOTE If we use global position of rechit,
          // 'inconsistent bin contents' exception may occur.
          me_detail_rec_occ_eta_[key3]->Fill(simhit_global_eta);
          me_detail_rec_occ_phi_[key3]->Fill(simhit_global_phi);

          if(chamber_id == 1) {
            me_detail_occ_xy_ch1_[key3]->Fill(rechit_global_x, rechit_global_y);
          }

        }

        // If we find a RecHit that matches a given SimHit,
        // then break the loop over RecHits and go to the next SimHit.
        break;

      } // matched if block
    } // end loop over rechits
  } // end loop over simhits

}
