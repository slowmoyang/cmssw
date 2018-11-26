#include "Validation/MuonGEMDigis/interface/GEMCoPadDigiValidation.h"

#include "DataFormats/GEMDigi/interface/GEMCoPadDigiCollection.h"

#include "TMath.h"


GEMCoPadDigiValidation::GEMCoPadDigiValidation(const edm::ParameterSet& ps)
    : GEMBaseValidation(ps) {

  auto copad_label = ps.getParameter<edm::InputTag>("copadLabel");
  copad_token_ = consumes<GEMCoPadDigiCollection>(copad_label);

  // FIXME
  minBXGEM_ = ps.getParameter<Int_t>("minBXGEM"); 
  maxBXGEM_ = ps.getParameter<Int_t>("maxBXGEM"); 
}


void GEMCoPadDigiValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                            edm::Run const & run,
                                            edm::EventSetup const & event_setup) {

  const GEMGeometry* kGEM = initGeometry(event_setup);
  if ( kGEM == nullptr) return ;

  ibooker.setCurrentFolder(folder_);

  for(const auto &  region : kGEM->regions()) {
    Int_t region_id = region->region();

    me_occ_zr_[region_id] = bookZROccupancy(ibooker, region_id, "copad", "CoPad");

    for(const auto & station : region->stations() ) {
      Int_t station_id = station->station();
      Int_t num_pads = station->superChambers()[0]->chambers()[0]->etaPartitions()[0]->npads();

      ME2IdsKey key2(region_id, station_id);

      me_occ_det_[key2] = bookDetectorOccupancy(ibooker, key2, station, "copad", "CoPad");

      if (detail_plot_) {
        me_detail_occ_zr_[key2] = bookZROccupancy(ibooker, key2, "copad_dg", "CoPad Digi");
        me_detail_occ_xy_[key2] = bookXYOccupancy(ibooker, key2, "copad_dg","CoPad Digi");

        me_detail_occ_phi_pad_[key2] = bookHist2D(
            ibooker, key2,
            "copad_digi_occ_phi_pad",
            "CoPad Digi Occupancy",
            280, -M_PI, M_PI,
            num_pads / 2, 0, num_pads,
            "#phi [rad]", "Pad number");

        me_detail_occ_pad_[key2] = bookHist1D(
            ibooker, key2,
            "copad_digi_occ_pad",
            "CoPad Digi Ocupancy per pad number",
            num_pads, 0.5, num_pads + 0.5,
            "Pad number");

        me_detail_bx_[key2] = bookHist1D(
            ibooker, key2,
            "copad_digi_bx",
            "CoPad Digi Bunch Crossing",
            11, -5.5, 5.5,
            "bunch crossing");
      }
    } // end loop over station ids
  } // end loop over region ids
}


GEMCoPadDigiValidation::~GEMCoPadDigiValidation() {


}


void GEMCoPadDigiValidation::analyze(const edm::Event& event,
                                     const edm::EventSetup& event_setup) {
  const GEMGeometry* kGEM = initGeometry(event_setup);
  if (kGEM == nullptr) return ;

  edm::Handle<GEMCoPadDigiCollection> copad_collection;
  event.getByToken(copad_token_, copad_collection);
  if (not copad_collection.isValid()) {
    edm::LogError(log_category_) << "Cannot get pads by token.";
    return ;
  }

  // GEMCoPadDigiCollection::DigiRangeIterator
  for (auto range_iter = copad_collection->begin();
            range_iter != copad_collection->end();
            range_iter++) {

    GEMDetId gem_id = (*range_iter).first;
    const GEMCoPadDigiCollection::Range& range = (*range_iter).second;

    Int_t region_id  = gem_id.region();
    Int_t station_id = gem_id.station();
    Int_t ring_id    = gem_id.ring();
    Int_t layer_id   = gem_id.layer();
    Int_t chamber_id = gem_id.chamber();

    ME2IdsKey key2(region_id, station_id);

    // GEMCoPadDigiCollection::const_iterator digi;
    for (auto digi = range.first; digi != range.second; ++digi) {

      // GEM copads are stored per super chamber!
      // layer_id = 0, roll_id = 0
      GEMDetId super_chamber_id = GEMDetId(region_id, ring_id, station_id, 0, chamber_id, 0);
      Int_t roll_id = (*digi).roll();

      const GeomDet* geom_det = kGEM->idToDet(super_chamber_id);
      if ( geom_det == nullptr) {
        edm::LogError(log_category_) << super_chamber_id << " : This detId cannot be "
                                     << "loaded from GEMGeometry // Original"
                                     << gem_id << " station : " << station_id << std::endl
                                     << "Getting DetId failed. Discard this gem copad hit."
                                     << std::endl;
        continue; 
      }

      const BoundPlane & surface = geom_det->surface();
      const GEMSuperChamber * superChamber = kGEM->superChamber(super_chamber_id);

      Int_t pad1 = digi->pad(1);
      Int_t pad2 = digi->pad(2);
      Int_t bx1  = digi->bx(1);
      Int_t bx2  = digi->bx(2);

      // Filtered using BX
      if ( bx1 < minBXGEM_ or bx1 > maxBXGEM_) continue;
      if ( bx2 < minBXGEM_ or bx2 > maxBXGEM_) continue;

      //  const GEMChamber* chamber(nnt layer) const;
      LocalPoint lp1 = superChamber->chamber(1)->etaPartition(roll_id)->centreOfPad(pad1);
      LocalPoint lp2 = superChamber->chamber(2)->etaPartition(roll_id)->centreOfPad(pad2);

      GlobalPoint gp1 = surface.toGlobal(lp1);
      GlobalPoint gp2 = surface.toGlobal(lp2);

      Float_t g_r1 =  gp1.perp();
      Float_t g_r2 =  gp2.perp();

      Float_t g_z1 =  gp1.z();
      Float_t g_z2 =  gp2.z();

      Float_t g_phi =  gp1.phi();
      Float_t g_x =  gp1.x();
      Float_t g_y =  gp1.y();

      // Fill normal plots.
      me_occ_zr_[region_id]->Fill(std::fabs(g_z1), g_r1);
      me_occ_zr_[region_id]->Fill(std::fabs(g_z2), g_r2);

      Int_t bin_x = getDetOccBinX(chamber_id, layer_id);
      me_occ_det_[key2]->Fill(bin_x, roll_id);
      me_occ_det_[key2]->Fill(bin_x + 1, roll_id);

      // Fill detail plots.
      if (detail_plot_) {
        me_detail_occ_xy_[key2]->Fill(g_x, g_y);     
        me_detail_occ_phi_pad_[key2]->Fill(g_phi, pad1);
        me_detail_occ_pad_[key2]->Fill(pad1);
        me_detail_bx_[key2]->Fill(bx1);
        me_detail_bx_[key2]->Fill(bx2);
        me_detail_occ_zr_[key2]->Fill(std::fabs(g_z1), g_r1);
        me_detail_occ_zr_[key2]->Fill(std::fabs(g_z2), g_r2);
      } // detailPlot_ 
    }
  }
}
