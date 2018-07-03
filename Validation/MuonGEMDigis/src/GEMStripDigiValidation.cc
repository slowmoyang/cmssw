#include "Validation/MuonGEMDigis/interface/GEMStripDigiValidation.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"

#include <TMath.h>
#include <iomanip>


GEMStripDigiValidation::GEMStripDigiValidation(const edm::ParameterSet& cfg): GEMBaseValidation(cfg) {
  InputTagToken_ = consumes<GEMDigiCollection>(cfg.getParameter<edm::InputTag>("stripLabel"));
  detailPlot_ = cfg.getParameter<bool>("detailPlot");

  nStripsGE11_ = cfg.getUntrackedParameter< int >("nStripsGE11");
  nStripsGE21_ = cfg.getUntrackedParameter< int >("nStripsGE21");

}

void GEMStripDigiValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                            edm::Run const & Run,
                                            edm::EventSetup const & iSetup )
{
    
  const GEMGeometry* GEMGeometry_  = initGeometry(iSetup);
  if ( GEMGeometry_ == nullptr)
  {
    edm::LogError("GEMStripDigiValidation") << "Failed to initialise GEMGeometry\n";
    return ;
  }

  LogDebug("GEMStripDigiValidation") << "Geometry is acquired from MuonGeometryRecord\n";
  ibooker.setCurrentFolder("MuonGEMDigisV/GEMDigisTask");
  LogDebug("GEMStripDigiValidation") << "ibooker set current folder\n";

  for(auto& region : GEMGeometry_->regions()) {
    int region_id = region->region();
    unsigned ridx = getRegionIndex(region_id);

    if(auto* tmp_zr = bookZROccupancy(ibooker, "strip", "Strip", region_id))
      me_occ_zr_[ridx] = tmp_zr;
    else
      edm::LogError("GEMStripDigiValidation") << "cannot book ";  // TODO

    for(auto& station : region->stations()) {
      int station_id = station->station();
      unsigned sidx = getStationIndex(station_id);

      if(auto* tmp_det = bookDetectorOccupancy(ibooker, station, "strip", "Strip", region_id))
        me_occ_det_[ridx][sidx] = tmp_det; 
      else
        edm::LogError("GEMStripDigiValidation") << "cannot book ";  // TODO
    }
  }


  // Booking detail plot.
  if(detailPlot_) {
    for(auto& region : GEMGeometry_->regions()) {
      int region_id = region->region();
      unsigned ridx = getRegionIndex(region_id);

      for( auto& station : region->stations()) {
        int station_id = station->station();
        unsigned sidx = getStationIndex(station_id);

        int num_strips = station_id == 1 ? nStripsGE11_ : nStripsGE21_;

        for(int layer_id : {1, 2}) {
          unsigned lidx = getLayerIndex(layer_id);

          me_detail_occ_zr_[ridx][sidx][lidx] = bookZROccupancy(ibooker, kMENamePrefix_, "Strip Digi", region_id, station_id, layer_id);
          me_detail_occ_xy_[ridx][sidx][lidx] = bookXYOccupancy(ibooker, kMENamePrefix_, "Strip Digi", region_id, station_id, layer_id);

          const char* name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
          const char* title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id);

          TString name, title;

          name = TString::Format("%s_occ_phi_strip_%s", kMENamePrefix_, name_suffix);
          title = TString::Format("DIGI Occupancy %s; #phi [rad];strip number", title_suffix);
          me_detail_occ_phi_strip_[ridx][sidx][lidx] = ibooker.book2D(name, title, 280, -TMath::Pi(), TMath::Pi(), num_strips/2, 0, num_strips);

          name = TString::Format("%s_occ_strip_%s", kMENamePrefix_, name_suffix);
          title = TString::Format("DIGI Occupancy per stip number: %s;strip number; entries", title_suffix);
          me_detail_occ_strip_[ridx][sidx][lidx] = ibooker.book1D(name, title, num_strips, 0.5, num_strips+0.5);

          name = TString::Format("%s_bunch_crossing_%s", kMENamePrefix_, name_suffix);
          title = TString::Format("Bunch crossing: %s; bunch crossing ; entries", title_suffix);
          me_detail_bx_[ridx][sidx][lidx] = ibooker.book1D(name, title, 11, -5.5, 5.5);

          name = TString::Format("%s_occ_xy_%s_even", kMENamePrefix_, name_suffix);
          title = TString::Format("Digi XY occupancy %s at even chambers", title_suffix);
          me_detail_occ_xy_chamber_[ridx][sidx][lidx][0] = ibooker.book2D(name, title, 360, -360, 360, 360, -360, 360);

          name = TString::Format("%s_occ_xy_%s_odd", kMENamePrefix_, name_suffix);
          title = TString::Format("Digi XY occupancy %s at odd chambers", title_suffix);
          me_detail_occ_xy_chamber_[ridx][sidx][lidx][1] = ibooker.book2D(name, title, 360, -360, 360, 360, -360, 360);

        } // Layer Loop End
      } // Station Loop End
    } // Region Loop End
  } // detailPlot if End

  LogDebug("GEMStripDigiValidation")<<"Booking End.\n";
}


GEMStripDigiValidation::~GEMStripDigiValidation() {
}


void GEMStripDigiValidation::analyze(const edm::Event& e,
                                     const edm::EventSetup& iSetup)
{
  // TODO unify 
  const GEMGeometry* GEMGeometry_ = initGeometry(iSetup);

  edm::Handle<GEMDigiCollection> gem_digis;
  e.getByToken( this->InputTagToken_, gem_digis);
  if (not gem_digis.isValid()) {
    edm::LogError("GEMStripDigiValidation") << "Cannot get strips by Token stripToken.\n";
    return ;
  }


  for (GEMDigiCollection::DigiRangeIterator cItr=gem_digis->begin(); cItr!=gem_digis->end(); cItr++)
  {
    GEMDetId id = (*cItr).first;

    const GeomDet* gdet = GEMGeometry_->idToDet(id);
    if ( gdet == nullptr)
    { 
      std::cout<<"Getting DetId failed. Discard this gem strip hit.Maybe it comes from unmatched geometry."<<std::endl;
      continue; 
    }

    const BoundPlane & surface = gdet->surface();
    const GEMEtaPartition * roll = GEMGeometry_->etaPartition(id);

    int region_id = id.region();
    int layer_id = id.layer();
    int station_id = id.station();
    Short_t chamber = (Short_t) id.chamber();
    Short_t nroll = (Short_t) id.roll();

    unsigned ridx = getRegionIndex(region_id);
    unsigned sidx = getStationIndex(station_id);
    unsigned lidx = getLayerIndex(layer_id);

    GEMDigiCollection::const_iterator digiItr;
    for (digiItr = (*cItr ).second.first; digiItr != (*cItr ).second.second; ++digiItr)
    {
      Short_t strip = (Short_t) digiItr->strip();
      Short_t bx = (Short_t) digiItr->bx();

      LocalPoint lp = roll->centreOfStrip(digiItr->strip());

      GlobalPoint gp = surface.toGlobal(lp);
      Float_t g_r = (Float_t) gp.perp();
      //Float_t g_eta = (Float_t) gp.eta();
      Float_t g_phi = (Float_t) gp.phi();
      Float_t g_x = (Float_t) gp.x();
      Float_t g_y = (Float_t) gp.y();
      Float_t g_z = (Float_t) gp.z();


      int binX = (chamber-1)*2+lidx;
      int binY = nroll;

      // Fill normal plots.
      me_occ_zr_[ridx]->Fill(std::fabs(g_z), g_r);
      me_occ_det_[ridx][sidx]->Fill( binX, binY); 

      // Fill detail plots.
      if ( detailPlot_)
      {
        me_detail_occ_zr_[ridx][sidx][lidx]->Fill(g_z,g_r);
        me_detail_occ_xy_[ridx][sidx][lidx]->Fill(g_x,g_y);     
        me_detail_occ_phi_strip_[ridx][sidx][lidx]->Fill(g_phi,strip);
        me_detail_occ_strip_[ridx][sidx][lidx]->Fill(strip);
        me_detail_bx_[ridx][sidx][lidx]->Fill(bx);

        auto chamber_parity = static_cast<unsigned>(chamber);
        me_detail_occ_xy_chamber_[ridx][sidx][lidx][chamber_parity]->Fill(g_x,g_y);
      } // detailPlot_
    }    
  }
}
