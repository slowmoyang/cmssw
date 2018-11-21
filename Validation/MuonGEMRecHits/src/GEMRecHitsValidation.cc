#include "Validation/MuonGEMRecHits/interface/GEMRecHitsValidation.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonDetId/interface/MuonSubdetId.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixPropagator.h"
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"


GEMRecHitsValidation::GEMRecHitsValidation(const edm::ParameterSet& ps)
    : GEMBaseValidation(ps) {
  auto muon_label = ps.getParameter<edm::InputTag>("muonLabel");
  muon_token_ = consumes<edm::View<reco::Muon> >(muon_label);

  auto service_params = ps.getParameter<edm::ParameterSet>("ServiceParameters");
  muon_service_proxy_ = new MuonServiceProxy(service_params);

  muon_pt_cut_ = ps.getParameter<Double_t>("muonpTCut");
}


GEMRecHitsValidation::~GEMRecHitsValidation() {
}


void GEMRecHitsValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                          edm::Run const & run,
                                          edm::EventSetup const & event_setup) {
  const GEMGeometry* kGEM = initGeometry(event_setup);
  if (kGEM == nullptr) return; 
  ibooker.setCurrentFolder(folder_);

  me_cls_ = ibooker.book1D(
      "cls_tot",
      "Cluster Size Distribution;The number of adjacent strips;Entries",
      10, 0.5, 10.5);

  // FIXME Chamber matching
  for (const auto & region : kGEM->regions()) {
    Int_t region_id = region->region();

    me_rechit_occ_zr_[region_id] = bookZROccupancy(ibooker, region_id, "rechit", "RecHit");

    me_residual_x_[region_id] = bookHist1D(ibooker, region_id,
                                           "residual_x", "Residual in Local X",
                                           100, -10.0, 10.0,
                                           "Residuals in Local X [cm]");
    me_residual_y_[region_id] = bookHist1D(ibooker, region_id,
                                           "residual_y", "Residual in Local Y",
                                           100, -10.0, 10.0,
                                           "Residual in Local Y [cm]");
    me_pull_x_[region_id] = bookHist1D(ibooker, region_id,
                                       "pull_x", "Pull in Local X",
                                       100, -5.0, 5.0);
    me_pull_y_[region_id] = bookHist1D(ibooker, region_id,
                                       "pull_y", "Pull in Local Y",
                                       100, -5.0, 5.0);


    // For efficiency
    me_muon_occ_eta_[region_id] = bookHist1D(
        ibooker, region_id,
        "muon_occ_eta", "Muon Eta Occupancy",
        50, eta_range_[0], eta_range_[1], "|#eta|");

    me_rechit_occ_eta_[region_id] = bookHist1D(
        ibooker, region_id,
        "rechit_occ_eta", "RecHit Eta Occupancy",
        50, eta_range_[0], eta_range_[1], "|#eta|");

    if (detail_plot_) {
      // NOTE Eta Occupancy
      me_detail_muon_occ_eta_.first[region_id] = bookHist1D(
          ibooker, region_id,
          "loose_muon_occ_eta",
          "Loose Muon Eta Occupancy (Cut Based Id)",
          51, eta_range_[0], eta_range_[1], "|#eta|");

      me_detail_muon_occ_eta_.second[region_id] = bookHist1D(
          ibooker, region_id,
          "tight_muon_occ_eta",
          "Tight Muon Eta Occupancy (Cut Based Id)",
          51, eta_range_[0], eta_range_[1], "|#eta|");

      me_detail_rechit_occ_eta_.first[region_id] = bookHist1D(
          ibooker, region_id,
          "loose_rechit_occ_eta",
          "Eta occupancy of RecHit matched with loose muon",
          51, eta_range_[0], eta_range_[1], "|#eta|");

      me_detail_rechit_occ_eta_.second[region_id] = bookHist1D(
          ibooker, region_id,
          "tight_rechit_occ_eta",
          "Eta occupancy of RecHit matched with tight muon",
          51, eta_range_[0], eta_range_[1], "|#eta|");

    }

    for (const auto & station : region->stations()) {
      Int_t station_id = station->station();
      ME2IdsKey key2(region_id, station_id);

      const GEMSuperChamber* super_chamber = station->superChambers().front();

      // NOTE Phi Ocuupancy for efficiency
      me_muon_occ_phi_[key2] = bookHist1D(
          ibooker, key2,
          "muon_occ_phi", "Muon Phi Occupancy",
          51, -M_PI, M_PI, "#phi [rad]");
      me_rechit_occ_phi_[key2] = bookHist1D(
          ibooker, key2,
          "rechit_occ_phi", "RecHit Phi Occupancy",
          51, -M_PI, M_PI, "#phi [rad]");

      /*
      // FIXME Chamber Matching
      me_muon_occ_phi_[key2] = bookHist1D(
          ibooker, key2,
          "ch_muon_occ_phi", "Muon Phi Occupancy (Chamber Matching)",
          51, -M_PI, M_PI, "#phi [rad]");
      me_rechit_occ_phi_[key2] = bookHist1D(
          ibooker, key2,
          "ch_rechit_occ_phi", "RecHit Phi Occupancy (Chamber Matching)",
          51, -M_PI, M_PI, "#phi [rad]");
      */

      // NOTE Detector Component Occupancy for efficienc
      me_muon_occ_det_[key2] = bookDetectorOccupancy(
          ibooker, key2, station, "muon", "Muon");
      me_rechit_occ_det_[key2] = bookDetectorOccupancy(
          ibooker, key2, station, "rechit", "RecHit");

    // NOTE Residual and Pull Plots with Cluster Size
    Int_t num_rolls = super_chamber->chambers().front()->nEtaPartitions();

    me_residual_x_roll_[key2] = bookHist2D(
        ibooker, key2,
        "residual_x_roll", "Residual in Local X",
        100, -10.0, 10.0,
        num_rolls, 0.5, num_rolls + 0.5,
        "Residuals in Local X [cm]", "i#eta");

    me_residual_y_roll_[key2] = bookHist2D(
        ibooker, key2,
        "residual_y_roll", "Residual in Local Y",
        100, -10.0, 10.0,
        num_rolls, 0.5, num_rolls + 0.5,
        "Residual in Local Y [cm]", "i#eta");

    me_pull_x_roll_[key2] = bookHist2D(
        ibooker, key2,
        "pull_x_roll", "Pull in Local X",
        100, -5.0, 5.0,
        num_rolls, 0.5, num_rolls + 0.5,
        "Pull in Local Y", "i#eta");

    me_pull_y_roll_[key2] = bookHist2D(
        ibooker, key2,
        "pull_y_roll", "Pull in Local Y",
        100, -5.0, 5.0,
        num_rolls, 0.5, num_rolls + 0.5,
        "Pull in Local Y", "i#eta");

    me_cls_roll_[key2] = bookHist2D(
        ibooker, key2,
        "cls_roll", "Cluster Size",
        10, 0.5, 10.5,
        num_rolls, 0.5, num_rolls + 0.5,
        "The number of adjacent strips", "i#eta");

      if (detail_plot_) {
        // NOTE Phi Ocuupancy for efficiency

        me_detail_muon_occ_phi_.first[key2] = bookHist1D(
            ibooker, key2,
            "loose_muon_occ_phi",
            "Loose Muon Phi Occupancy (Cut Based Id)",
            51, -M_PI, M_PI, "#phi [rad]");

        me_detail_muon_occ_phi_.second[key2] = bookHist1D(
            ibooker, key2,
            "tight_muon_occ_phi",
            "Tight Muon Phi Occupancy (Cut Based Id)",
            51, -M_PI, M_PI, "#phi [rad]");

        me_detail_rechit_occ_phi_.first[key2] = bookHist1D(
            ibooker, key2,
            "loose_rechit_occ_phi",
            "RecHit Phi Occupancy",
            51, -M_PI, M_PI, "#phi [rad]");

        me_detail_rechit_occ_phi_.second[key2] = bookHist1D(
            ibooker, key2,
            "tight_rechit_occ_phi",
            "RecHit Phi Occupancy",
            51, -M_PI, M_PI, "#phi [rad]");

        ///////////////////////////////////////////////////////////////////////
        // NOTE Detector Component Occupancy for efficienc
        ///////////////////////////////////////////////////////////////////////
        me_detail_muon_occ_det_.first[key2] = bookDetectorOccupancy(
            ibooker, key2, station, "loose_muon", "Loose Muon");

        me_detail_muon_occ_det_.second[key2] = bookDetectorOccupancy(
            ibooker, key2, station, "tight_muon", "Tight Muon");

        me_detail_rechit_occ_det_.first[key2] = bookDetectorOccupancy(
            ibooker, key2, station,
            "loose_rechit", "RecHit Matched to Loose Muon");

        me_detail_rechit_occ_det_.second[key2] = bookDetectorOccupancy(
            ibooker, key2, station,
            "tight_rechit", "RecHit Matched to Tight Muon");


      } // end detail plot


      for (const auto & chamber : super_chamber->chambers()) {
        Int_t layer_id = chamber->id().layer();
        ME3IdsKey key3(region_id, station_id, layer_id);

        if (detail_plot_) {
          me_detail_rechit_occ_zr_[key3] = bookZROccupancy(
              ibooker, key3, "rechit", "RecHits");

          me_detail_rechit_occ_xy_[key3] = bookXYOccupancy(
              ibooker, key3, "rechit", "RecHits");

          // NOTE check if geometry is fine
          me_detail_rechit_occ_xy_ch1_[key3] = bookHist2D(
              ibooker, key3,
              "rechit_occ_xy_ch1",
              "RecHit XY Occupancy (Chamber 1 Only);X [cm];Y [cm]",
              xy_occ_num_bins_, -360, 360,
              xy_occ_num_bins_, -360, 360);
              
          me_detail_rechit_occ_polar_[key3] = bookPolarOccupancy(
              ibooker, key3, "rechit", "recHits");

          // bookHist1D(ibooker, name, title, nbinsx, xlow, xup, x_title)
          // NOTE include xup
          me_detail_cls_[key3] = bookHist1D(
              ibooker, key3,
              "cls", "Cluster Size Distribution",
              10, 0.5, 10.5, "The number of adjacent strips");

          // Occupancy histograms of SimHits and RecHits for Efficiency
          me_detail_residual_x_[key3] = bookHist1D(
              ibooker, key3,
              "residual_x", "Residual in Local X",
              100, -5, 5, "Residual in Local X [cm]");

          me_detail_residual_y_[key3] = bookHist1D(
              ibooker, key3,
              "residual_y", "Residual in Local Y",
              100, -5, 5, "Residual in Local Y [cm]");

          me_detail_pull_x_[key3] = bookHist1D(
              ibooker, key3, "pull_x", "Pull in Local X", 100, -5, 5);

          me_detail_pull_y_[key3] = bookHist1D(
              ibooker, key3, "pull_y", "Pull in Local Y", 100, -5, 5);

        }
      } // end loop over layers
    } // station loop
  } // region loop



  return;
}


const GEMEtaPartition* GEMRecHitsValidation::findEtaPartition(
    const GEMChamber * chamber,
    GlobalPoint & global_point) {

  const GEMEtaPartition *matched_roll = nullptr;
  for (const auto & roll : chamber->etaPartitions()) {

    LocalPoint && local_point = roll->toLocal(global_point);
    Local2DPoint local_2d_point(local_point.x(), local_point.y());

    const Bounds & bounds = roll->surface().bounds();
    if (bounds.inside(local_2d_point)) {
      matched_roll = roll;
      break;
    }
  }

  return matched_roll;
}


void GEMRecHitsValidation::analyze(const edm::Event& event,
                                   const edm::EventSetup& event_setup) {

  //////////////////////////////////
  // Event
  ///////////////////////////////////
  edm::Handle<edm::View<reco::Muon> > muons;
  event.getByToken(muon_token_, muons);

  ////////////////////////////
  // EventSetup
  ////////////////////////
  const GEMGeometry* kGEM = initGeometry(event_setup);
  if (kGEM == nullptr) return; 

  edm::ESHandle<TransientTrackBuilder> transient_track_builder;
  event_setup.get<TransientTrackRecord>().get("TransientTrackBuilder", transient_track_builder);
  muon_service_proxy_->update(event_setup);
  edm::ESHandle<Propagator> propagator = muon_service_proxy_->propagator(kPropagatorName_);

  ///////////////////////////////////////////////////////////
  // Muon
  ///////////////////////////////////////////////////////////
  for(size_t muon_idx = 0; muon_idx < muons->size(); ++muon_idx) {
    edm::RefToBase<reco::Muon> muon_ref = muons->refAt(muon_idx);
    const reco::Muon *muon = muon_ref.get();

    if (muon->pt() < muon_pt_cut_) continue;

    Int_t muon_id = 0;
    if (muon->passed(reco::Muon::Selector::CutBasedIdTight)) {
      muon_id = kTightId_;
    } else if (muon->passed(reco::Muon::Selector::CutBasedIdLoose)) {
      muon_id = kLooseId_;
    }

    const reco::Track *muon_track = nullptr;  
    if (muon->globalTrack().isNonnull()) {
      muon_track = muon->globalTrack().get();
    } else if (muon->outerTrack().isNonnull()) {
      muon_track = muon->outerTrack().get();
    } else {
      edm::LogInfo(log_category_) << "no global and outer tracks" << std::endl;
      continue;
    }

    reco::TransientTrack && transient_track = transient_track_builder->build(muon_track);

    // Find GEMRecHits
    std::vector<const GEMRecHit*> gem_rechits;
    for (trackingRecHit_iterator iter = muon_track->recHitsBegin();
                                 iter != muon_track->recHitsEnd();
                                 iter++) {
      
      if (not (*iter)->isValid()) {
        edm::LogInfo(log_category_) << "invalid tracking rechit" << std::endl;
        continue;
      }

      // Check if TrackingRecHit is the GEMRecHit
      DetId && rechit_detid = (*iter)->geographicalId();
      if (rechit_detid.det() != DetId::Detector::Muon) continue;
      if (rechit_detid.subdetId() != MuonSubdetId::GEM) continue;

      auto rechit = dynamic_cast<const GEMRecHit*>(*iter);
      gem_rechits.push_back(rechit);
    }

    /////////////////////////////
    // NOTE
    ////////////////////////////////////
    for (const auto & chamber : kGEM->chambers()) {
      // TODO write comment
      if (muon->eta() * chamber->id().region() < 0) continue;


      TrajectoryStateOnSurface && traj_state = propagator->propagate(
          transient_track.outermostMeasurementState(),
          chamber->surface());

      if (not traj_state.isValid()) continue;

      GlobalPoint && traj_global_pos = traj_state.globalPosition();
      const GEMEtaPartition* && roll = findEtaPartition(chamber, traj_global_pos);
      if (roll == nullptr) {
        edm::LogInfo(log_category_) << "TrajectoryStateOnSurface is valid. "
                                    << "But failed to find GEMEtaPartition"
                                    << std::endl;
        continue;
      }

      GEMDetId traj_gemid = roll->id();

      Int_t region_id  = traj_gemid.region();
      Int_t station_id = traj_gemid.station();
      Int_t layer_id   = traj_gemid.layer();
      Int_t chamber_id = traj_gemid.chamber();
      Int_t roll_id    = traj_gemid.roll();

      ME2IdsKey key2(region_id, station_id);
      ME3IdsKey key3(region_id, station_id, layer_id);

      // LocalPoint traj_local_pos = traj_state.localPosition();
      LocalPoint && traj_local_pos = roll->toLocal(traj_global_pos);

      if (not traj_state.hasError()) {
        edm::LogError(log_category_) << "TrajectoryStateOnSurface does't have error" << std::endl;
        continue;
      }
      LocalError && traj_local_err = traj_state.localError().positionError();

      // Track Global Position on a given chamber
      Float_t traj_g_eta = std::abs(traj_global_pos.eta());
      Float_t traj_g_phi = traj_global_pos.phi();
      // Track Local Position on a given chamber
      Float_t traj_l_x = traj_local_pos.x();
      Float_t traj_l_y = traj_local_pos.y();
      // bin x of detector occupancy plots
      Int_t det_occ_bin_x = getDetOccBinX(chamber_id, layer_id);

      me_muon_occ_eta_[region_id]->Fill(traj_g_eta);
      me_muon_occ_phi_[key2]->Fill(traj_g_phi);
      me_muon_occ_det_[key2]->Fill(det_occ_bin_x, roll_id);

      if (detail_plot_) {
        if (muon_id >= kLooseId_) {
          me_detail_muon_occ_eta_.first[region_id]->Fill(traj_g_eta);
          me_detail_muon_occ_phi_.first[key2]->Fill(traj_g_phi);
          me_detail_muon_occ_det_.first[key2]->Fill(det_occ_bin_x, roll_id);

          if (muon_id == kTightId_) {
          me_detail_muon_occ_eta_.second[region_id]->Fill(traj_g_eta);
          me_detail_muon_occ_phi_.second[key2]->Fill(traj_g_phi);
          me_detail_muon_occ_det_.second[key2]->Fill(det_occ_bin_x, roll_id);
          }
        }

      }


      ////////////////////////////////////////////////
      // NOTE On Eta Partition
      ////////////////////////////////////////////////////
      Float_t closet_distance = std::numeric_limits<Float_t>::infinity();
      const GEMRecHit* matched_rechit = nullptr;
      for (const auto & rechit : gem_rechits) {
        if (traj_gemid != rechit->gemId()) continue;

        LocalError && rechit_local_err   = rechit->localPositionError();
        if (rechit_local_err.invalid()) {
          edm::LogError(log_category_) << "Invalid LocalError of GEMRecHit" << std::endl;
          continue;
        }

        LocalPoint && rechit_local_pos   = rechit->localPosition();

        //
        Float_t residual_x = rechit_local_pos.x() - traj_l_x;
        Float_t residual_y = rechit_local_pos.y() - traj_l_y;

        Float_t distance = std::hypot(residual_x, residual_y);

        if (distance < closet_distance) {
          closet_distance = distance;
          matched_rechit = rechit;
        }

      }


      if (matched_rechit == nullptr) continue;

      LocalPoint && rechit_local_pos   = matched_rechit->localPosition();
      GlobalPoint && rechit_global_pos = roll->toGlobal(rechit_local_pos);
      LocalError && rechit_local_err = matched_rechit->localPositionError();

      Int_t cls = matched_rechit->clusterSize();

      // local coordinates of RecHit
      Float_t rechit_l_x = rechit_local_pos.x();
      Float_t rechit_l_y = rechit_local_pos.y();
      // global coordinates of RecHit
      Float_t rechit_g_x   = rechit_global_pos.x();
      Float_t rechit_g_y   = rechit_global_pos.y();
      Float_t rechit_g_z   = std::abs(rechit_global_pos.z());
      Float_t rechit_g_r   = rechit_global_pos.perp();
      // Float_t rechit_g_eta = std::abs(rechit_global_pos.eta());
      Float_t rechit_g_phi = rechit_global_pos.phi();

      //
      Float_t residual_x = rechit_l_x - traj_l_x;
      Float_t residual_y = rechit_l_y - traj_l_y;

      Float_t resolution_x = std::sqrt(rechit_local_err.xx() + traj_local_err.xx());
      Float_t resolution_y = std::sqrt(rechit_local_err.yy() + traj_local_err.yy());

      Float_t pull_x = residual_x / resolution_x;
      Float_t pull_y = residual_y / resolution_y;

      me_cls_->Fill(cls);

      me_rechit_occ_zr_[region_id]->Fill(rechit_g_z, rechit_g_r);
  
      me_residual_x_[region_id]->Fill(residual_x);
      me_residual_y_[region_id]->Fill(residual_y);
      me_pull_x_[region_id]->Fill(pull_x);
      me_pull_y_[region_id]->Fill(pull_y);

      me_residual_x_roll_[key2]->Fill(residual_x, roll_id);
      me_residual_y_roll_[key2]->Fill(residual_y, roll_id);
      me_pull_x_roll_[key2]->Fill(pull_x, roll_id);
      me_pull_y_roll_[key2]->Fill(pull_y, roll_id);

      me_cls_roll_[key2]->Fill(cls, roll_id);

      // FIXME Even if muon and rechit are matched, they can be filled in
      // different bins. Therefore, the rechit occupancy is also filled
      // with muon's one.
      me_rechit_occ_eta_[region_id]->Fill(traj_g_eta);
      me_rechit_occ_phi_[key2]->Fill(traj_g_phi);
      me_rechit_occ_det_[key2]->Fill(det_occ_bin_x, roll_id);

      if (detail_plot_) {
        me_detail_cls_[key3]->Fill(cls);

        me_detail_rechit_occ_xy_[key3]->Fill(rechit_g_x, rechit_g_y);
        me_detail_rechit_occ_xy_ch1_[key3]->Fill(rechit_g_x, rechit_g_y);
        me_detail_rechit_occ_zr_[key3]->Fill(rechit_g_z, rechit_g_r);
        me_detail_rechit_occ_polar_[key3]->Fill(rechit_g_phi, rechit_g_r);

        me_detail_residual_x_[key3]->Fill(residual_x);
        me_detail_residual_y_[key3]->Fill(residual_y);
        me_detail_pull_x_[key3]->Fill(pull_x);
        me_detail_pull_y_[key3]->Fill(pull_y);

        if (muon_id >= kLooseId_) {
          me_detail_rechit_occ_eta_.first[region_id]->Fill(traj_g_eta);
          me_detail_rechit_occ_phi_.first[key2]->Fill(traj_g_phi);
          me_detail_rechit_occ_det_.first[key2]->Fill(det_occ_bin_x, roll_id);

          if (muon_id == kTightId_) {
            me_detail_rechit_occ_eta_.second[region_id]->Fill(traj_g_eta);
            me_detail_rechit_occ_phi_.second[key2]->Fill(traj_g_phi);
            me_detail_rechit_occ_det_.second[key2]->Fill(det_occ_bin_x, roll_id);
          }
        }

      } // detail plot

    } // end loop over GEM chambers (trajectory state on surface)
  } // end loop over muon

}
