#include "Validation/MuonGEMHits/interface/GEMHitsValidation.h"

#include "DataFormats/Common/interface/Handle.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"


GEMHitsValidation::GEMHitsValidation(const edm::ParameterSet& ps)
    : GEMBaseValidation(ps) {
  auto simhit_label = ps.getParameter<edm::InputTag>("simhitLabel");
  simhit_token_ = consumes<edm::PSimHitContainer>(simhit_label);

  // time of flight
  tof_range_ = ps.getUntrackedParameter<std::vector<Double_t> >("TOFRange");
}


GEMHitsValidation::~GEMHitsValidation() {
}


void GEMHitsValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                       edm::Run const & run,
                                       edm::EventSetup const & event_setup) {
  const GEMGeometry* kGEM = initGeometry(event_setup);
  if (kGEM == nullptr) return;

  ibooker.setCurrentFolder(folder_);

  // NOTE region-independent
  for(const auto & station : kGEM->regions()[0]->stations()) {
    Int_t station_id = station->station();

    // time of flight
    Double_t tof_min, tof_max;
    std::tie(tof_min, tof_max) = getTOFRange(station_id);

    const char* tof_name  = TString::Format("tof_muon_st%d", station_id).Data();
    const char* tof_title = TString::Format(
        "SimHit TOF (Muon only) : Station %d;Time of flight [ns];entries",
        station_id).Data();
    me_tof_mu_[station_id] = ibooker.book1D(tof_name, tof_title,
                                            40, tof_min, tof_max);

    const char* eloss_name  = TString::Format("eloss_muon_st%d", station_id).Data();
    const char* eloss_title = TString::Format(
        "SimHit Energy Loss (Muon only) : Station %d;Energy loss [eV];entries",
        station_id).Data();
    me_eloss_mu_[station_id] = ibooker.book1D(eloss_name, eloss_title,
                                              60, 0.0, 6000.0);
  } // end loop over stations


  for (const auto & region : kGEM->regions()) {
    Int_t region_id = region->region();

    me_occ_zr_[region_id] = bookZROccupancy(ibooker, region_id, "simhit", "SimHit");

    for (const auto & station : region->stations()) {
      Int_t station_id = station->station();
      ME2IdsKey key2(region_id, station_id);
    
      Double_t tof_min, tof_max;
      std::tie(tof_min, tof_max) = getTOFRange(station_id);

      me_occ_det_[key2] = bookDetectorOccupancy(ibooker, key2, station, "simhit", "SimHit");

      const GEMSuperChamber* super_chamber = station->superChambers().front();
      for (const auto & chamber : super_chamber->chambers()) {
        Int_t layer_id = chamber->id().layer();
        ME3IdsKey key3(region_id, station_id, layer_id);

        if (detail_plot_) {
          me_detail_occ_zr_[key3] = bookZROccupancy(ibooker, key3, "simhit", "SimHit");
          me_detail_occ_xy_[key3] = bookXYOccupancy(ibooker, key3, "simhit", "SimHit");

          me_detail_tof_[key3] = bookHist1D(
              ibooker, key3,
              "tof", "SimHit TOF",
              40, tof_min, tof_max,
              "Time of Flight [ns]");

          me_detail_tof_mu_[key3] = bookHist1D(
              ibooker, key3,
              "tof_muon", "SimHit TOF (Muon only)",
              40, tof_min, tof_max,
              "Time of Flight [ns]");

          me_detail_eloss_[key3] = bookHist1D(
              ibooker, key3,
              "eloss", "SimHit Energy Loss",
              60, 0.0, 6000.0,
              "Energy los [eV]");

          me_detail_eloss_mu_[key3] = bookHist1D(
              ibooker, key3,
              "eloss_muon", "SimHit Energy Loss (Muon Only)",
              60, 0.0, 6000.0,
              "Energy loss [eV]");

        } // end if-statement
      } // end loop over layer ids
    } // end loop over station ids
  } // end loop over retion ids

  return;
}


std::tuple<Double_t, Double_t> GEMHitsValidation::getTOFRange(Int_t station_id) {
  UInt_t start_index = station_id == 1 ? 0 : 2;
  Double_t tof_min = tof_range_[start_index];
  Double_t tof_max = tof_range_[start_index + 1];
  return std::make_tuple(tof_min, tof_max);
}


void GEMHitsValidation::analyze(const edm::Event & event,
                                const edm::EventSetup & event_setup) {

  const GEMGeometry* kGEM = initGeometry(event_setup) ;
  if (kGEM == nullptr) return;

  edm::Handle<edm::PSimHitContainer> simhit_container;
  event.getByToken(simhit_token_, simhit_container);
  if (not simhit_container.isValid()) {
    edm::LogError(log_category_) << "Cannot get GEMHits by Token simInputTagToken";
    return ;
  }

  for (const auto & simhit : *simhit_container.product()) {
    const GEMDetId gemid(simhit.detUnitId());

    if (kGEM->idToDet(gemid) == nullptr) {
      edm::LogInfo(log_category_) << "SimHit did not matched with GEM Geometry." << std::endl;
      continue;
    }

    Int_t region_id  = gemid.region();
    Int_t station_id = gemid.station();
    Int_t layer_id   = gemid.layer();
    Int_t chamber_id = gemid.chamber();
    Int_t roll_id    = gemid.roll(); // eta partition

    ME2IdsKey key2(region_id, station_id);
    ME3IdsKey key3(region_id, station_id, layer_id);

    LocalPoint && simhit_local_pos = simhit.localPosition();
    GlobalPoint && simhit_global_pos = kGEM->idToDet(gemid)->surface().toGlobal(simhit_local_pos);

    Float_t simhit_g_r = simhit_global_pos.perp();
    Float_t simhit_g_x = simhit_global_pos.x();
    Float_t simhit_g_y = simhit_global_pos.y();
    Float_t simhit_g_abs_z = std::fabs(simhit_global_pos.z());

    Float_t energy_loss = kEnergyCF_ * simhit.energyLoss();
    Float_t tof = simhit.timeOfFlight();

    // SimplePlot
    me_occ_zr_[region_id]->Fill(simhit_g_abs_z, simhit_g_r);
    Int_t bin_x = getDetOccBinX(chamber_id, layer_id);
    me_occ_det_[key2]->Fill(bin_x, roll_id);

    if (std::abs(simhit.particleType()) == kMuonPDGId_) {
      me_tof_mu_[station_id]->Fill(tof);
      me_eloss_mu_[station_id]->Fill(energy_loss);
    }

    if (detail_plot_) {
      // First, fill variable has no condition.
      me_detail_occ_zr_[key3]->Fill(simhit_g_abs_z, simhit_g_r);
      me_detail_occ_xy_[key3]->Fill(simhit_g_x, simhit_g_y);

      me_detail_tof_[key3]->Fill(tof);
      me_detail_eloss_[key3]->Fill(energy_loss);

      if (std::abs(simhit.particleType()) == kMuonPDGId_) {
        me_detail_tof_mu_[key3]->Fill(tof);
        me_detail_eloss_mu_[key3]->Fill(energy_loss);
      }

    } // detailPlot

  } // end loop over simhits
}
