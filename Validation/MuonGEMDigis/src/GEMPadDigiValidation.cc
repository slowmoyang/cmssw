#include "Validation/MuonGEMDigis/interface/GEMPadDigiValidation.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"
#include <TMath.h>

GEMPadDigiValidation::GEMPadDigiValidation(const edm::ParameterSet& cfg): GEMBaseValidation(cfg)
{
  InputTagToken_ = consumes<GEMPadDigiCollection>(cfg.getParameter<edm::InputTag>("PadLabel"));
  detailPlot_ = cfg.getParameter<bool>("detailPlot");
}


void GEMPadDigiValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                          edm::Run const & Run,
                                          edm::EventSetup const & iSetup)
{
    
  const GEMGeometry* GEMGeometry_ = initGeometry(iSetup);
  if ( GEMGeometry_ == nullptr) return ;
  LogDebug("GEMPadDigiValidation")<<"Geometry is acquired from MuonGeometryRecord\n";
  ibooker.setCurrentFolder("MuonGEMDigisV/GEMDigisTask");
  LogDebug("GEMPadDigiValidation")<<"ibooker set current folder\n";

  if ( GEMGeometry_ == nullptr) return ;

  int npadsGE11 = GEMGeometry_->regions()[0]->stations()[0]->superChambers()[0]->chambers()[0]->etaPartitions()[0]->npads();
  int npadsGE21 = 0;

  if (GEMGeometry_->regions()[0]->stations().size()>1 && !GEMGeometry_->regions()[0]->stations()[1]->superChambers().empty() )
  {
    npadsGE21 = GEMGeometry_->regions()[0]->stations()[1]->superChambers()[0]->chambers()[0]->etaPartitions()[0]->npads();
  }


  // TODO Debug
  for(auto& region : GEMGeometry_->regions())
  {
    int region_id = region->region();
    unsigned ridx = getRegionIndex(region_id);

    if(auto* simpleZR = bookZROccupancy(ibooker, "pad_digi", "Pad DIGI", region_id))
      me_occ_zr_[ridx] = simpleZR;
    // TODO LogError 

    for( auto& station : region->stations())
    {
      int station_id = station->station();
      unsigned sidx = getStationIndex(station_id);

      if(auto* dcEta = bookDetectorOccupancy(ibooker, station, "pad_digi", "Pad DIGI", region_id))
        me_occ_det_[ridx][sidx] = dcEta;
        // TODO LogError

    } // STATION LOOP END
  } // REGION LOOP END


  if (detailPlot_)
  {
    for( auto& region : GEMGeometry_->regions() )
    {
      int region_id = region->region();
      unsigned ridx = getRegionIndex(region_id);

      for( auto& station : region->stations() )
      {
        int station_id = station->station();
        unsigned sidx = getStationIndex(station_id);

        int nPads = station_id == 1 ? npadsGE11 : npadsGE21;

        for(int layer_id : {1, 2})
        {
          unsigned lidx = getLayerIndex(layer_id);

          me_detail_occ_zr_[ridx][sidx][lidx] = bookZROccupancy(ibooker, "pad_dg", "Pad Digi", region_id, station_id , layer_id);
          me_detail_occ_xy_[ridx][sidx][lidx] = bookXYOccupancy(ibooker, "pad_dg", "Pad Digi", region_id, station_id , layer_id);

          const char* name_suffix  = GEMUtils::getSuffixName(region_id, station_id, layer_id).Data();
          const char* title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id).Data();

          TString name, title;

          name = TString::Format("pad_dg_phipad%s", name_suffix);
          title = TString::Format("Digi occupancy:%s; phi [rad]; Pad number", title_suffix);
          me_detail_occ_phi_pad_[ridx][sidx][lidx] = ibooker.book2D(name, title, 280,-TMath::Pi(),TMath::Pi(), nPads/2,0,nPads );


          name = TString::Format("pad_dg%s", name_suffix);
          title = TString::Format("Digi occupancy per pad number:%s;Pad number; entries", title_suffix);
          me_detail_occ_pad_[ridx][sidx][lidx] = ibooker.book1D(name, title, nPads, 0.5, nPads + 0.5);

          name = TString::Format("pad_dg_bx%s", name_suffix);
          title = TString::Format("Bunch crossing:%s; bunch crossing ; entries", title_suffix);
          me_detail_bx_[ridx][sidx][lidx] = ibooker.book1D(name, title, 11,-5.5,5.5);

          name = TString::Format("pad_dg_xy%s_even", name_suffix);
          title = TString::Format("Digi XY occupancy %s at even chambers", title_suffix);
          me_detail_occ_xy_chamber_[ridx][sidx][lidx][0] = ibooker.book2D(name, title, 360, -360,360, 360, -360, 360);

          name = TString::Format("pad_dg_xy%s_odd", name_suffix);
          title = TString::Format("Digi XY occupancy %s at odd chambers", title_suffix);
          me_detail_occ_xy_chamber_[ridx][sidx][lidx][1] = ibooker.book2D(name, title, 360, -360,360, 360, -360, 360);


        } // layer loop end
      } // station loop end
    } // region loop END
  } // detailPlot END
}


GEMPadDigiValidation::~GEMPadDigiValidation() { }


void GEMPadDigiValidation::analyze(const edm::Event& e,
                                   const edm::EventSetup& iSetup)
{
  // FIXME unify
  const GEMGeometry* GEMGeometry_ = initGeometry(iSetup);

  // typedef MuonDigiCollection<GEMDetId, GEMPadDigi> GEMPadDigiCollection;
  edm::Handle<GEMPadDigiCollection> gem_digis;
  e.getByToken(InputTagToken_, gem_digis);
  if (not gem_digis.isValid())
  {
    edm::LogError("GEMPadDigiValidation") << "Cannot get pads by label GEMPadToken.";
    return;
  }


  //for(const auto & pad_digi : gem_digis)
  for (GEMPadDigiCollection::DigiRangeIterator pad_digi=gem_digis->begin(); pad_digi!=gem_digis->end(); pad_digi++)
  {
    GEMDetId id = (*pad_digi).first;
    const GeomDet* gdet = GEMGeometry_->idToDet(id);

    if ( gdet == nullptr)
    { 
      std::cout<<"Getting DetId failed. Discard this gem pad hit.Maybe it comes from unmatched geometry."<<std::endl;
      continue; 
    }

    const BoundPlane & surface = gdet->surface();
    const GEMEtaPartition * roll = GEMGeometry_->etaPartition(id);

    int region_id = id.region();
    int station_id = id.station();
    int layer_id = id.layer();
    int chamber_id = id.chamber();
    int roll_id = id.roll();

    unsigned ridx = getRegionIndex(region_id);
    unsigned sidx = getStationIndex(station_id);
    unsigned lidx = getLayerIndex(layer_id);

    //loop over digis of given roll
    GEMPadDigiCollection::const_iterator digiItr;
    for (digiItr = (*pad_digi ).second.first; digiItr != (*pad_digi ).second.second; ++digiItr)
    {
      Short_t pad = (Short_t) digiItr->pad();
      Short_t bx = (Short_t) digiItr->bx();

      LocalPoint lp = roll->centreOfPad(digiItr->pad());

      GlobalPoint gp = surface.toGlobal(lp);
      Float_t g_r = (Float_t) gp.perp();
      Float_t g_phi = (Float_t) gp.phi();
      Float_t g_x = (Float_t) gp.x();
      Float_t g_y = (Float_t) gp.y();
      Float_t g_z = (Float_t) gp.z();

      edm::LogInfo("GEMPadDIGIValidation")<<"Global x "<<g_x<<"Global y "<<g_y<<"\n";  
      edm::LogInfo("GEMPadDIGIValidation")<<"Global pad "<<pad<<"Global phi "<<g_phi<<std::endl; 
      edm::LogInfo("GEMPadDIGIValidation")<<"Global bx "<<bx<<std::endl; 

      int binX = (chamber_id-1)*2+lidx;
      int binY = roll_id;

      // Fill normal plots.
      me_occ_zr_[ridx]->Fill(std::fabs(g_z), g_r);
      me_occ_det_[ridx][sidx]->Fill( binX, binY); 

      if ( detailPlot_)
      {
        me_detail_occ_xy_[ridx][sidx][lidx]->Fill(g_x,g_y);     
        me_detail_occ_phi_pad_[ridx][sidx][lidx]->Fill(g_phi,pad);
        me_detail_occ_pad_[ridx][sidx][lidx]->Fill(pad);
        me_detail_bx_[ridx][sidx][lidx]->Fill(bx);
        me_detail_occ_zr_[ridx][sidx][lidx]->Fill(g_z,g_r);

        me_detail_occ_xy_chamber_[ridx][sidx][lidx][static_cast<unsigned>(chamber_id%2)]->Fill(g_x,g_y);
      }
    }
  }
}
