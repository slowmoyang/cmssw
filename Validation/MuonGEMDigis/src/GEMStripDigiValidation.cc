#include "Validation/MuonGEMDigis/interface/GEMStripDigiValidation.h"

#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

#include "Geometry/CommonTopologies/interface/StripTopology.h"

#include <TMath.h>
#include <iomanip>


GEMStripDigiValidation::GEMStripDigiValidation(const edm::ParameterSet & ps)
    : GEMBaseValidation(ps) {

  auto strip_label = ps.getParameter<edm::InputTag>("stripLabel");
  strip_token_ = consumes<GEMDigiCollection>(strip_label);

  auto simhit_label = ps.getParameter<edm::InputTag>("simhitLabel");
  simhit_token_ = consumes<edm::PSimHitContainer>(simhit_label);
}


void GEMStripDigiValidation::bookHistograms(DQMStore::IBooker & booker,
                                            edm::Run const & run,
                                            edm::EventSetup const & event_setup) {

  const GEMGeometry* gem = initGeometry(event_setup);

  // NOTE Occupancy
  const char* occ_folder = gSystem->ConcatFileName(folder_.c_str(), "Occupancy");
  booker.setCurrentFolder(occ_folder);

  for (const auto & region : gem->regions()) {
    Int_t region_id = region->region();

    me_occ_zr_[region_id] = bookZROccupancy(booker, region_id, "strip", "Strip");

    // NOTE occupancy plots for eta efficiency
    me_simhit_occ_eta_[region_id] = bookHist1D(
        booker, region_id, "muon_simhit_occ_eta", "Muon SimHit Eta Occupancy",
        50, eta_range_[0], eta_range_[1], "|#eta|");

    me_strip_occ_eta_[region_id] = bookHist1D(
        booker, region_id, "matched_strip_occ_eta", "Strip Digi Eta Occupancy",
        50, eta_range_[0], eta_range_[1], "|#eta|");

    for (const auto & station : region->stations()) {
      Int_t station_id = station->station();
      ME2IdsKey key2(region_id, station_id);

      me_occ_det_[key2] = bookDetectorOccupancy(booker, key2, station, "strip", "Strip Digi");

      // NOTE occupancy plots for phi efficiency
      me_simhit_occ_phi_[key2] = bookHist1D(
          booker, key2, "muon_simhit_occ_phi", "Muon SimHit Phi Occupancy",
          51, -M_PI, M_PI, "#phi");

      me_strip_occ_phi_[key2] = bookHist1D(
          booker, key2, "matched_strip_occ_phi", "Matched Digi Phi Occupancy",
          51, -M_PI, M_PI, "#phi");

      // NOTE occupancy plots for detector component efficiency
      me_simhit_occ_det_[key2] = bookDetectorOccupancy(
          booker, key2, station, "muon_simhit", "Muon SimHit");
      me_strip_occ_det_[key2] = bookDetectorOccupancy(
          booker, key2, station, "matched_strip", "Matched Strip Digi");

      const GEMSuperChamber* super_chamber = station->superChambers().front();
      for (const auto & chamber : super_chamber->chambers()) {
        Int_t layer_id = chamber->id().layer();
        ME3IdsKey key3(region_id, station_id, layer_id);

        Int_t num_strips = chamber->etaPartitions().front()->nstrips();

        if (detail_plot_) {
          me_detail_occ_xy_[key3] = bookXYOccupancy(booker, key3, "strip", "Strip Digi");

          me_detail_occ_phi_strip_[key3] = bookHist2D(
              booker, key3, "strip_occ_phi_strip", "Strip Digi Occupancy",
              280, -M_PI, M_PI, num_strips / 2, 0, num_strips,
              "#phi [rad]", "strip number");

          me_detail_occ_strip_[key3] = bookHist1D(
              booker, key3, "strip_occ_strip", "Strip Digi Occupancy per strip number",
              num_strips, 0.5, num_strips + 0.5, "strip number");

        }
      } // End loop over layer ids
    } // End loop over station ids
  } // End loop over region ids


  // NOTE Bunch Crossing
  if (detail_plot_) {
    const char* bx_folder = gSystem->ConcatFileName(folder_.c_str(), "BunchCrossing");
    booker.setCurrentFolder(bx_folder);

    for (const auto & region : gem->regions()) {
      Int_t region_id = region->region();
      for (const auto & station : region->stations()) {
        Int_t station_id = station->station();

        const GEMSuperChamber* super_chamber = station->superChambers().front();
        for (const auto & chamber : super_chamber->chambers()) {
          Int_t layer_id = chamber->id().layer();
          ME3IdsKey key3(region_id, station_id, layer_id);

            me_detail_bx_[key3] = bookHist1D(
                booker, key3, "strip_bx", "Strip Digi Bunch Crossing",
                5, -2.5, 2.5, "Bunch crossing");

        } // chamber loop
      } // station loop
    } // region loop
  } // detail plot

}


GEMStripDigiValidation::~GEMStripDigiValidation() {
}


void GEMStripDigiValidation::analyze(const edm::Event & event,
                                     const edm::EventSetup & event_setup) {

  const GEMGeometry* gem = initGeometry(event_setup);

  edm::Handle<edm::PSimHitContainer> simhit_container;
  event.getByToken(simhit_token_, simhit_container);
  if (not simhit_container.isValid()) {
    edm::LogError(log_category_) << "Failed to get PSimHitContainer." << std::endl;
    return ;
  }

  edm::Handle<GEMDigiCollection> digi_collection;
  event.getByToken(strip_token_, digi_collection);
  if (not digi_collection.isValid()) {
    edm::LogError(log_category_) << "Cannot get strips by Token stripToken." << std::endl;
    return ;
  }

  for (auto range_iter = digi_collection->begin();
            range_iter != digi_collection->end();
            range_iter++) {

    GEMDetId id = (*range_iter).first;
    const GEMDigiCollection::Range& range = (*range_iter).second;

    if (gem->idToDet(id) == nullptr) { 
      edm::LogError(log_category_) << "Getting DetId failed. Discard this gem strip hit. "
                                   << "Maybe it comes from unmatched geometry."
                                   << std::endl;
      continue; 
    }

    const BoundPlane & surface = gem->idToDet(id)->surface();
    const GEMEtaPartition* roll = gem->etaPartition(id);

    Int_t region_id  = id.region();
    Int_t layer_id   = id.layer();
    Int_t station_id = id.station();
    Int_t chamber_id = id.chamber();
    Int_t roll_id    = id.roll();

    // keys for MonitorElement* map.
    ME2IdsKey key2(region_id, station_id);
    ME3IdsKey key3(region_id, station_id, layer_id);
    Int_t bin_x = getDetOccBinX(chamber_id, layer_id);

    for (auto digi = range.first; digi != range.second; ++digi) {
      Int_t strip = digi->strip();
      Int_t bx = digi->bx();

      LocalPoint strip_local_pos = roll->centreOfStrip(digi->strip());
      GlobalPoint strip_global_pos = surface.toGlobal(strip_local_pos);

      Float_t g_r   = strip_global_pos.perp();
      Float_t g_phi = strip_global_pos.phi();
      Float_t g_x   = strip_global_pos.x();
      Float_t g_y   = strip_global_pos.y();
      Float_t g_abs_z   = std::abs(strip_global_pos.z());

      // Simple Plots
      me_occ_zr_[region_id]->Fill(g_abs_z, g_r);
      me_occ_det_[key2]->Fill(bin_x, roll_id);

      // Detail Plots
      if (detail_plot_) {
        me_detail_occ_xy_[key3]->Fill(g_x, g_y);     
        me_detail_occ_phi_strip_[key3]->Fill(g_phi, strip);
        me_detail_occ_strip_[key3]->Fill(strip);
        me_detail_bx_[key3]->Fill(bx);
      } // detailPlot_ if end

    } // digi loop end
  } // end loop over digi_collection

  //////////////////////////////////////////////////////////////////////////////
  // TODO if (is_mc_)
  //////////////////////////////////////////////////////////////////////////////
  for (const auto & simhit : *simhit_container.product()) {
    // muon only
    if (not isMuonSimHit(simhit)) continue;

    if (gem->idToDet(simhit.detUnitId()) == nullptr) {
      edm::LogError(log_category_) << "SimHit did not match with GEMGeometry." << std::endl;
      continue;
    }

    GEMDetId simhit_gemid(simhit.detUnitId());

    Int_t region_id = simhit_gemid.region();
    Int_t station_id = simhit_gemid.station();
    Int_t layer_id = simhit_gemid.layer();
    Int_t chamber_id = simhit_gemid.chamber();
    Int_t roll_id = simhit_gemid.roll();

    ME2IdsKey key2(region_id, station_id);
    ME3IdsKey key3(region_id, station_id, layer_id);

    const GEMEtaPartition* roll = gem->etaPartition(simhit_gemid);

    LocalPoint && simhit_local_pos = simhit.localPosition();
    GlobalPoint && simhit_global_pos = roll->surface().toGlobal(simhit_local_pos);

    Float_t simhit_g_eta = std::abs(simhit_global_pos.eta());
    Float_t simhit_g_phi = simhit_global_pos.phi();

    Int_t simhit_strip = roll->strip(simhit_local_pos);

    Int_t bin_x = getDetOccBinX(chamber_id, layer_id);
    me_simhit_occ_eta_[region_id]->Fill(simhit_g_eta);
    me_simhit_occ_phi_[key2]->Fill(simhit_g_phi);
    me_simhit_occ_det_[key2]->Fill(bin_x, roll_id); 

    Bool_t found_matched_digi = false;

    // GEMCoPadDigiCollection::DigiRangeIterator
    for (auto range_iter = digi_collection->begin();
              range_iter != digi_collection->end();
              range_iter++) {

      if (simhit_gemid != (*range_iter).first) continue;

      const GEMDigiCollection::Range& range = (*range_iter).second;
      for (auto digi = range.first; digi != range.second; ++digi) {

        if (simhit_strip == digi->strip()) {
          found_matched_digi = true;

          me_strip_occ_eta_[region_id]->Fill(simhit_g_eta);
          me_strip_occ_phi_[key2]->Fill(simhit_g_phi);
          me_strip_occ_det_[key2]->Fill(bin_x, roll_id); 

          break;
        }
      } // end loop over range

      if (found_matched_digi) break;

    } // end lopp over digi_collection
  } // end loop over simhit_container
}
