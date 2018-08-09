#include "Validation/MuonGEMHits/interface/GEMHitsValidation.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"

#include "DataFormats/Common/interface/Handle.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"

// ROOT classes
#include "TSystem.h"

// std classes
#include <exception>

using namespace std;

GEMHitsValidation::GEMHitsValidation(const edm::ParameterSet& ps) : GEMBaseValidation(ps) {
  auto simhit_label = ps.getParameter<edm::InputTag>("simInputLabel");
  SimHitToken_ = consumes<edm::PSimHitContainer>(simhit_label);

  TOFRange_ = ps.getUntrackedParameter<std::vector<Double_t> >("TOFRange");
  detailPlot_ = ps.getParameter<Bool_t>("detailPlot");
  folder_ = ps.getParameter<std::string>("folder");
}


GEMHitsValidation::~GEMHitsValidation() {
}


void GEMHitsValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                       edm::Run const & Run,
                                       edm::EventSetup const & iSetup) {
  const GEMGeometry* kGEM = initGeometry(iSetup);
  if ( kGEM == nullptr) {
    edm::LogError(kLogCategory_) << "Cannot initialise GEMGeometry in the "
                                       << "bookHistograms step.\n";
    return;
  }

  ibooker.setCurrentFolder(folder_);

  // NOTE region-independent
  for(const auto & station : kGEM->regions()[0]->stations()) {
    Int_t station_id = station->station();

    // FIXME structured binding..
    Double_t tof_min, tof_max;
    std::tie(tof_min, tof_max) = getTOFRange(station_id);

    // FIXME too long..
    const char* name_tof  = TString::Format("simhit_tof_mu_st%d", station_id).Data();
    const char* title_tof = TString::Format("SimHit TOF (Muon only) : Station %d;Time of flight [ns];entries", station_id).Data();
    me_tof_mu_[station_id] = ibooker.book1D(name_tof, title_tof, 40, tof_min, tof_max);

    const char* name_eloss  = TString::Format("simhit_eloss_mu_st%d", station_id).Data();
    const char* title_eloss = TString::Format("SimHit Energy Loss (Muon only) : Station %d;Energy loss [eV];entries", station_id).Data();
    me_eloss_mu_[station_id] = ibooker.book1D(name_eloss, title_eloss, 60, 0.0, 6000.0);
  } // STATION LOOP END


  // Regions, Region+station
  for(const auto & region : kGEM->regions()) {
    Int_t region_id = region->region();
    me_occ_zr_[region_id] = bookZROccupancy(ibooker, region_id, "simhit", "SimHit");

    for(const auto & station : region->stations()) {
      Int_t station_id = station->station();
      ME2IdsKey key2(region_id, station_id);
      me_occ_det_[key2] = bookDetectorOccupancy(ibooker, key2, station, "simhit", "SimHit");
    } // end loop over station ids;
  } // end loop over region ids


  if(detailPlot_ ) {
    for(const auto & region : kGEM->regions()) {
      Int_t region_id = region->region();

      for(const auto & station : region->stations()) {
        Int_t station_id = station->station();

        Double_t tof_min, tof_max;
        std::tie(tof_min, tof_max) = getTOFRange(station_id);

        // FIXME
        for(Int_t layer_id : {1, 2}) {

          ME3IdsKey key(region_id, station_id, layer_id);

          me_detail_occ_zr_[key] = bookZROccupancy(ibooker, key, "simhit", "SimHit");
          me_detail_occ_xy_[key] = bookXYOccupancy(ibooker, key, "simhit", "SimHit");

          me_detail_tof_[key] = bookHist1D(
              ibooker, key,
              "tof", "SimHit TOF",
              40, tof_min, tof_max,
              "Time of Flight [ns]", "entries");

          me_detail_tof_mu_[key] = bookHist1D(
              ibooker, key,
              "tof_muon", "SimHit TOF (Muon only)",
              40, tof_min, tof_max,
              "Time of Flight [ns]", "entries");

          me_detail_eloss_[key] = bookHist1D(
              ibooker, key,
              "eloss", "SimHit Energy Loss",
              60, 0.0, 6000.0,
              "Energy los [eV]", "entries");

          me_detail_eloss_mu_[key] = bookHist1D(
              ibooker, key,
              "eloss_muon", "SimHit Energy Loss (Muon Only)",
              60, 0., 6000.0,
              "Energy loss [eV]", "entries");

        } // end loop over layer ids
      } // end loop over station ids
    } // end loop over retion ids
  } // detailPlot IF END

  // NOTE
  me_gem_geom_xyz_ = ibooker.book3D(
      "gem_geom_xyz",
      "GEM Roll Position;x [cm];y [cm];z [cm]",
      160, -800.0, 800.0,
      160, -800.0, 800.0, 
      240, -1200.0, 1200.0);

  me_gem_geom_eta_phi_ = ibooker.book2D(
      "gem_geom_eta_phi",
      "GEM Roll Position; #eta; #phi",
      101, -4, 4,
      101, -TMath::Pi(), TMath::Pi());

  // NOTE This histogram does not need to be filled repeatedly.
  const LocalPoint kLocalOrigin(0.0, 0.0, 0.0);
  for(const auto & det_id : kGEM->detUnitIds()) {
    GEMDetId kGEMId(det_id);

    const GEMEtaPartition* kEtaPartition = kGEM->etaPartition(kGEMId);
    GlobalPoint gp = kEtaPartition->toGlobal(kLocalOrigin);

    me_gem_geom_xyz_->Fill(gp.x(), gp.y(), gp.z());
    me_gem_geom_eta_phi_->Fill(gp.eta(), gp.phi());
  }

  // FIXME debugging

  std::string debug_dir = gSystem->ConcatFileName(folder_.c_str(), "debug");
  ibooker.setCurrentFolder(debug_dir);

  for(const auto & station : kGEM->regions()[0]->stations()) {
    Int_t station_id = station->station();

    me_debug_segment_x_[station_id] = ibooker.book1D(
        TString::Format("debug_segment_xi_st%d", station_id).Data(),
        TString::Format("Segment X : Station %d;exit.x - entry.x [cm];entries", station_id).Data(),
        800, -0.4, -0.4);
  }

  ibooker.setCurrentFolder(folder_);
  return;
}


std::tuple<Double_t, Double_t> GEMHitsValidation::getTOFRange(Int_t station_id) {
  UInt_t start_index = station_id == 1 ? 0 : 2;
  Double_t tof_min = TOFRange_[start_index];
  Double_t tof_max = TOFRange_[start_index + 1];
  edm::LogInfo(kLogCategory_) << "Exit getTOFRange." << std::endl;
  return std::make_tuple(tof_min, tof_max);
}


void GEMHitsValidation::analyze(const edm::Event& e,
                                const edm::EventSetup& iSetup) {

  const GEMGeometry* kGEM = initGeometry(iSetup) ;
  if(kGEM == nullptr) {
    edm::LogError(kLogCategory_) << "Failed to init GEMGeometry." << std::endl; 
    return ;
  }

  edm::Handle<edm::PSimHitContainer> simhit_container;
  e.getByToken(SimHitToken_, simhit_container);

  if (not simhit_container.isValid()) {
    edm::LogError(kLogCategory_) << "Cannot get GEMHits by Token simInputTagToken";
    return ;
  }

  for(const auto & simhit : *simhit_container.product()) {
    const GEMDetId kGEMId(simhit.detUnitId());

    if ( kGEM->idToDet(kGEMId) == nullptr) {
      // NOTE LogInfo or LogError
      edm::LogInfo(kLogCategory_) << "simHit did not matched with GEMGeometry.\n";
      continue;
    }

    // FIXME follow naiming convention
    const Int_t region_id  = kGEMId.region();
    const Int_t station_id = kGEMId.station();
    const Int_t layer_id   = kGEMId.layer();
    const Int_t chamber_id = kGEMId.chamber();
    const Int_t roll_id    = kGEMId.roll(); // eta partition

    const ME2IdsKey key2(region_id, station_id);
    const ME3IdsKey key3(region_id, station_id, layer_id);
    const ME4IdsKey key4(region_id, station_id, layer_id, roll_id);

    const LocalPoint kSimHitLocal = simhit.localPosition();
    const GlobalPoint kSimHitGlobal = kGEM->idToDet(kGEMId)->surface().toGlobal(kSimHitLocal);

    Float_t g_r = kSimHitGlobal.perp();
    Float_t g_x = kSimHitGlobal.x();
    Float_t g_y = kSimHitGlobal.y();
    Float_t g_abs_z = std::fabs(kSimHitGlobal.z());

    Float_t energy_loss = kEnergyCF_ * simhit.energyLoss();
    Float_t tof = simhit.timeOfFlight();

    me_occ_zr_[region_id]->Fill(g_abs_z, g_r);

    Int_t bin_x = getDetOccBinX(chamber_id, layer_id);
    me_occ_det_[key2]->Fill(bin_x, roll_id);

    if (std::abs(simhit.particleType()) == kMuonPDGId_) {
      me_tof_mu_[station_id]->Fill(tof);
      me_eloss_mu_[station_id]->Fill(energy_loss);

      // FIXME debugging
      Float_t segment_x = simhit.exitPoint().x() - simhit.entryPoint().x();
      me_debug_segment_x_[station_id]->Fill(segment_x);
    }

    if( detailPlot_ ) {
      // First, fill variable has no condition.
      me_detail_occ_zr_[key3]->Fill(g_abs_z, g_r);
      me_detail_occ_xy_[key3]->Fill(g_x, g_y);

      me_detail_tof_[key3]->Fill(tof);
      me_detail_eloss_[key3]->Fill(energy_loss);

      if (std::abs(simhit.particleType()) == kMuonPDGId_) {
        me_detail_tof_mu_[key3]->Fill(tof);
        me_detail_eloss_mu_[key3]->Fill(energy_loss);
      }

    } // detailPlot

  } // simhit loop END
}

