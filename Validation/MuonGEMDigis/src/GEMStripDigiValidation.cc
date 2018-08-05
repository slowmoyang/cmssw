#include "Validation/MuonGEMDigis/interface/GEMStripDigiValidation.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"

#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

#include "Geometry/CommonTopologies/interface/StripTopology.h"

#include <TMath.h>
#include <iomanip>


GEMStripDigiValidation::GEMStripDigiValidation(const edm::ParameterSet& ps): GEMBaseValidation(ps) {
  auto digi_label = ps.getParameter<edm::InputTag>("stripLabel");
  digi_token_ = consumes<GEMDigiCollection>(digi_label);

  auto simhit_label = ps.getParameter<edm::InputTag>("simHitLabel");
  simhit_token_ = consumes<edm::PSimHitContainer>(simhit_label);

  detailPlot_ = ps.getParameter<Bool_t>("detailPlot");

  nStripsGE11_ = ps.getUntrackedParameter<Int_t>("nStripsGE11");
  nStripsGE21_ = ps.getUntrackedParameter<Int_t>("nStripsGE21");

  folder_ = ps.getParameter<std::string>("folder");
}


void GEMStripDigiValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                            edm::Run const & Run,
                                            edm::EventSetup const & iSetup ) {
    
  const GEMGeometry* kGEM  = initGeometry(iSetup);
  if ( kGEM == nullptr) {
    edm::LogError(kLogCategory_) << "Failed to initialise kGEM\n";
    return ;
  }

  ibooker.setCurrentFolder(folder_);


  const Double_t kPi = TMath::Pi();

  for(const auto & region : kGEM->regions()) {
    Int_t region_id = region->region();

    if(auto tmp_zr = bookZROccupancy(ibooker, region_id, "strip", "Strip")) {
      me_occ_zr_[region_id] = tmp_zr;
    } else {
      edm::LogError(kLogCategory_) << "cannot book ";  // TODO
    }

    for(const auto & station : region->stations()) {
      Int_t station_id = station->station();
      ME2IdsKey key(region_id, station_id);

      if(auto tmp_me = bookDetectorOccupancy(ibooker, key, station, "strip", "Strip")) {
        me_occ_det_[key] = tmp_me;
      } else {
        edm::LogError(kLogCategory_) << "cannot book ";  // TODO
      }

    } // station loop end
  } // region loop end

  // Booking detail plot.
  if(detailPlot_) {
    for(const auto & region : kGEM->regions()) {
      Int_t region_id = region->region();

      for(const auto & station : region->stations()) {
        Int_t station_id = station->station();
        Int_t num_strips = station_id == 1 ? nStripsGE11_ : nStripsGE21_;

        for(Int_t layer_id : {1, 2}) {

          ME3IdsKey key3(region_id, station_id, layer_id);

          me_detail_occ_zr_[key3] = bookZROccupancy(ibooker, key3, "strip", "Strip Digi");
          me_detail_occ_xy_[key3] = bookXYOccupancy(ibooker, key3, "strip", "Strip Digi");

          me_detail_occ_phi_strip_[key3] = bookHist2D(ibooker, key3,
                                                      "strip_occ_phi_strip",
                                                      "Strip DIGI Occupancy",
                                                      280, -TMath::Pi(), TMath::Pi(),
                                                      num_strips / 2, 0, num_strips,
                                                      "#phi [rad]", "strip number");

          me_detail_occ_strip_[key3] = bookHist1D(ibooker, key3,
                                                  "strip_occ_strip",
                                                  "Strip DIGI Occupancy per strip number",
                                                  num_strips, 0.5, num_strips + 0.5,
                                                  "strip number");

          me_detail_bx_[key3] = bookHist1D(ibooker, key3,
                                           "strip_bx",
                                           "Strip DIGI Bunch Crossing",
                                           11, -5.5, 5.5,
                                           "bunch crossing");

          me_debug_simhit_occ_eta_[key3] = bookHist1D(
              ibooker, key3, "simhit_occ_eta", "SimHit Eta Occupancy",
              51, -4, 4, "#eta");

          me_debug_digi_occ_eta_[key3] = bookHist1D(
              ibooker, key3, "digi_occ_eta", "Digi Eta Occupancy",
              51, -4, 4, "#eta");

          me_debug_simhit_occ_phi_[key3] = bookHist1D(
              ibooker, key3, "simhit_occ_phi", "SimHit Phi Occupancy",
              51, -kPi, kPi, "#phi");

          me_debug_digi_occ_phi_[key3] = bookHist1D(
              ibooker, key3, "digi_occ_phi", "Digi Phi Occupancy",
              51, -kPi, kPi, "#phi");

          me_debug_unmatched_strip_diff_[key3] = bookHist1D(
              ibooker, key3, "debug_unmatched_strip_diff",
              "SimHit-DIGI Unmatched Case Strip Distance",
              -10, 10, "# of strips");

        } // End loop over layer ids
      } // End loop over station ids
    } // End loop over region ids
  } // detailPlot if End

  LogDebug("GEMStripDigiValidation")<<"Booking End.\n";
}


GEMStripDigiValidation::~GEMStripDigiValidation() {
}


void GEMStripDigiValidation::analyze(const edm::Event& e,
                                     const edm::EventSetup& iSetup) {

  const GEMGeometry* kGEM = initGeometry(iSetup);
  if ( kGEM == nullptr) {
    edm::LogError(kLogCategory_) << "Failed to initialise kGEM\n";
    return ;
  }

  edm::Handle<edm::PSimHitContainer> simhit_container;
  e.getByToken(simhit_token_, simhit_container);
  if (not simhit_container.isValid()) {
    edm::LogError(kLogCategory_) << "Failed to get PSimHitContainer.\n";
    return ;
  }

  edm::Handle<GEMDigiCollection> digi_collection;
  e.getByToken(digi_token_, digi_collection);
  if (not digi_collection.isValid()) {
    edm::LogError(kLogCategory_) << "Cannot get strips by Token stripToken.\n";
    return ;
  }

  for (auto range_iter = digi_collection->begin();
            range_iter != digi_collection->end();
            range_iter++) {

    GEMDetId id = (*range_iter).first;
    const GEMDigiCollection::Range& range = (*range_iter).second;

    const GeomDet* geom_det = kGEM->idToDet(id);
    if (geom_det == nullptr) { 
      std::cout << "Getting DetId failed. Discard this gem strip hit. "
                << "Maybe it comes from unmatched geometry."
                << std::endl;
      continue; 
    }

    const BoundPlane & surface = geom_det->surface();
    const GEMEtaPartition* roll = kGEM->etaPartition(id);

    Int_t region_id  = id.region();
    Int_t layer_id   = id.layer();
    Int_t station_id = id.station();
    Int_t chamber_id = id.chamber();
    Int_t roll_id    = id.roll();

    // keys for MonitorElement* map.
    ME2IdsKey key2(region_id, station_id);
    ME3IdsKey key3(region_id, station_id, layer_id);




    for (auto digi = range.first; digi != range.second; ++digi) {
      Int_t strip = digi->strip();
      Int_t bx = digi->bx();

      LocalPoint digi_local = roll->centreOfStrip(digi->strip());
      GlobalPoint digi_global = surface.toGlobal(digi_local);

      Float_t g_r   = digi_global.perp();
      Float_t g_phi = digi_global.phi();
      Float_t g_x   = digi_global.x();
      Float_t g_y   = digi_global.y();
      Float_t g_z   = digi_global.z();

      // Simple Plots
      me_occ_zr_[region_id]->Fill(std::fabs(g_z), g_r);

      Int_t bin_x = getDetOccBinX(chamber_id, layer_id);
      me_occ_det_[key2]->Fill(bin_x, roll_id); 

      // Detail Plots
      if ( detailPlot_) {
        me_detail_occ_zr_[key3]->Fill(std::fabs(g_z), g_r);
        me_detail_occ_xy_[key3]->Fill(g_x, g_y);     
        me_detail_occ_phi_strip_[key3]->Fill(g_phi, strip);
        me_detail_occ_strip_[key3]->Fill(strip);
        me_detail_bx_[key3]->Fill(bx);
      } // detailPlot_ if end

    } // digi loop end

  } // end loop over digi_collection


  //////////////////////////////////////
  // TODO 
  // NOTE 
  ///////////////////////////////////////
  for(const auto & simhit : *simhit_container.product()) {

    if (std::abs(simhit.particleType()) != kMuonPDGId_) {
      edm::LogInfo(kLogCategory_) << "PSimHit is not muon.\n";
      continue;
    }

    const UInt_t kSimDetUnitId = simhit.detUnitId();
    if (kGEM->idToDet(kSimDetUnitId) == nullptr) {
      // FIXME should I replace LogError as LogWarning or LogError?
      // NOTE
      edm::LogInfo(kLogCategory_) << "simHit did not matched with GEMGeometry.\n";
      continue;
    }
    const GEMDetId kSimHitGEMId(kSimDetUnitId);
    ME3IdsKey key3(kSimHitGEMId.region(), kSimHitGEMId.station(), kSimHitGEMId.layer());

    const LocalPoint kSimHitLocal = simhit.localPosition();
    const GlobalPoint kSimHitGlobal = kGEM->idToDet(kSimHitGEMId)->surface().toGlobal(kSimHitLocal);

    const Float_t kSimHitGlobalEta = kSimHitGlobal.eta();
    const Float_t kSimHitGlobalPhi = kSimHitGlobal.phi();

    Int_t kSimHitStrip = static_cast<Int_t>(std::ceil(kGEM->etaPartition(kSimDetUnitId)->strip(kSimHitLocal)));

    me_debug_simhit_occ_eta_[key3]->Fill(kSimHitGlobalEta);
    me_debug_simhit_occ_phi_[key3]->Fill(kSimHitGlobalPhi);

    Bool_t matched = false;
    Int_t min_strip_diff = 999;

    for (auto range_iter = digi_collection->begin();
              range_iter != digi_collection->end();
              range_iter++) {

      const GEMDetId kDigiGEMId = (*range_iter).first;
      if(kSimHitGEMId != kDigiGEMId) continue;

      const GEMDigiCollection::Range& range = (*range_iter).second;
      for (auto digi = range.first; digi != range.second; ++digi) {

        const Int_t kDigiStrip = digi->strip();

        if(kSimHitStrip == kDigiStrip) {
          matched = true;
          // const LocalPoint kDigiLocal = kGEM->etaPartition(kDigiGEMId)->centreOfStrip(digi->strip());
          // NOTE If we use global position of digi,
          // 'inconsistent bin contents' exception may occur.
          me_debug_digi_occ_eta_[key3]->Fill(kSimHitGlobalEta);
          me_debug_digi_occ_phi_[key3]->Fill(kSimHitGlobalPhi);
          break;
        } else {
          Int_t strip_diff = kDigiStrip - kSimHitStrip;
          if(strip_diff < min_strip_diff) {
            min_strip_diff = strip_diff;
          }
        }
      } // end loop over range

      if (matched) {
        break;
      } else {
        me_debug_unmatched_strip_diff_[key3]->Fill(min_strip_diff);
      }

    } // end lopp over digi_collection


  } // end loop over simhit_container




}
