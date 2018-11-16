#include "Validation/MuonGEMDigis/interface/GEMPadDigiValidation.h"
#include "DataFormats/GEMDigi/interface/GEMPadDigiCollection.h"


GEMPadDigiValidation::GEMPadDigiValidation(const edm::ParameterSet& ps)
    : GEMBaseValidation(ps) {
  auto label = ps.getParameter<edm::InputTag>("padLabel");
  pad_token_ = consumes<GEMPadDigiCollection>(label);
}


void GEMPadDigiValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                          edm::Run const & Run,
                                          edm::EventSetup const & event_setup) {
  const GEMGeometry* kGEM = initGeometry(event_setup);
  if (kGEM == nullptr) return ;

  ibooker.setCurrentFolder(folder_);

  for (const auto & region : kGEM->regions()) {
    Int_t region_id = region->region();

    me_occ_zr_[region_id] = bookZROccupancy(ibooker, region_id, "pad", "Pad DIGI");

    for (const auto & station : region->stations()) {
      Int_t station_id = station->station();
      ME2IdsKey key2(region_id, station_id);

      me_occ_det_[key2] = bookDetectorOccupancy(ibooker, key2, station, "pad", "Pad DIGI");

      const GEMSuperChamber* super_chamber = station->superChambers().front();
      for (const auto & chamber : super_chamber->chambers()) {
        Int_t layer_id = chamber->id().layer();
        ME3IdsKey key3(region_id, station_id, layer_id);

        Int_t num_pads = chamber->etaPartitions().front()->npads();

        if(detail_plot_) {
          me_detail_occ_zr_[key3] = bookZROccupancy(ibooker, key3, "pad", "Pad Digi");
          me_detail_occ_xy_[key3] = bookXYOccupancy(ibooker, key3, "pad", "Pad Digi");

          me_detail_occ_phi_pad_[key3] = bookHist2D(
              ibooker, key3,
              "occ_phi_pad",
              "Pad DIGI Occupancy",
              280, -M_PI, M_PI,
              num_pads / 2, 0, num_pads,
              "#phi [rad]", "Pad number");

          me_detail_occ_pad_[key3] = bookHist1D(
              ibooker, key3,
              "occ_pad",
              "Pad DIGI Occupancy",
              num_pads, -0.5, num_pads - 0.5,
              "GEM Pad Id");

          me_detail_bx_[key3] = bookHist1D(ibooker, key3,
                                           "bx", "Bunch Crossing",
                                           11, -5.5, 5.5, "Bunch Crossing");
        }
      } // end loop over layer ids
    } // end loop over station ids
  } // end loop over region ids
}


GEMPadDigiValidation::~GEMPadDigiValidation() { }


void GEMPadDigiValidation::analyze(const edm::Event & event,
                                   const edm::EventSetup & event_setup) {
  const GEMGeometry* kGEM = initGeometry(event_setup);
  if (kGEM == nullptr) return ;

  // typedef MuonDigiCollection<GEMDetId, GEMPadDigi> GEMPadDigiCollection;
  edm::Handle<GEMPadDigiCollection> collection;
  event.getByToken(pad_token_, collection);
  if (not collection.isValid()) {
    edm::LogError(log_category_) << "Cannot get pads by label GEMPadToken.";
    return;
  }

  // GEMPadDigiCollection::DigiRangeIterator;
  for (auto range_iter = collection->begin();
            range_iter != collection->end();
            range_iter++) {

    GEMDetId gemid = (*range_iter).first;
    const GEMPadDigiCollection::Range & range = (*range_iter).second;

    if (kGEM->idToDet(gemid) == nullptr) { 
      edm::LogError(log_category_) << "Getting DetId failed. Discard this gem pad hit. "
                                  << "Maybe it comes from unmatched geometry." << std::endl;
      continue; 
    }

    const GEMEtaPartition * roll = kGEM->etaPartition(gemid);
    const BoundPlane & surface = roll->surface();

    Int_t region_id  = gemid.region();
    Int_t station_id = gemid.station();
    Int_t layer_id   = gemid.layer();
    Int_t chamber_id = gemid.chamber();
    Int_t roll_id    = gemid.roll();

    ME2IdsKey key2(region_id, station_id);
    ME3IdsKey key3(region_id, station_id, layer_id);

    for (auto digi = range.first; digi != range.second; ++digi) {
      Int_t pad = digi->pad();
      Int_t bx  = digi->bx();

      LocalPoint && local_pos = roll->centreOfPad(pad);
      GlobalPoint && global_pos = surface.toGlobal(local_pos);

      Float_t g_r     = global_pos.perp();
      Float_t g_phi   = global_pos.phi();
      Float_t g_x     = global_pos.x();
      Float_t g_y     = global_pos.y();
      Float_t g_abs_z = std::fabs(global_pos.z());

      // Simple Plots
      me_occ_zr_[region_id]->Fill(g_abs_z, g_r);

      Int_t bin_x = getDetOccBinX(chamber_id, layer_id);
      me_occ_det_[key2]->Fill(bin_x, roll_id); 

      if (detail_plot_) {
        me_detail_occ_xy_[key3]->Fill(g_x, g_y);
        me_detail_occ_phi_pad_[key3]->Fill(g_phi, pad);
        me_detail_occ_pad_[key3]->Fill(pad);
        me_detail_bx_[key3]->Fill(bx);
        me_detail_occ_zr_[key3]->Fill(g_abs_z, g_r);
      } // detail_plot_

    }
  } // end loop over range iters
}
