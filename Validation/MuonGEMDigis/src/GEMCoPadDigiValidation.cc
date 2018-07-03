#include "Validation/MuonGEMDigis/interface/GEMCoPadDigiValidation.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"
#include <TMath.h>

GEMCoPadDigiValidation::GEMCoPadDigiValidation(const edm::ParameterSet& cfg): GEMBaseValidation(cfg)
{
  InputTagToken_ = consumes<GEMCoPadDigiCollection>(cfg.getParameter<edm::InputTag>("CopadLabel"));
  detailPlot_ = cfg.getParameter<bool>("detailPlot");
  minBXGEM_ = cfg.getParameter<int>("minBXGEM"); 
  maxBXGEM_ = cfg.getParameter<int>("maxBXGEM"); 
}


void GEMCoPadDigiValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                            edm::Run const & Run,
                                            edm::EventSetup const & iSetup)
{
  const GEMGeometry* GEMGeometry_ = initGeometry(iSetup);
  if ( GEMGeometry_ == nullptr) return ;


  LogDebug("GEMCoPadDigiValidation")<<"Geometry is acquired from MuonGeometryRecord\n";
  ibooker.setCurrentFolder("MuonGEMDigisV/GEMDigisTask");
  LogDebug("GEMCoPadDigiValidation")<<"ibooker set current folder\n";

  const double PI = TMath::Pi();

  int npadsGE11 = GEMGeometry_->regions()[0]->stations()[0]->superChambers()[0]->chambers()[0]->etaPartitions()[0]->npads();
  int npadsGE21 = 0;

  if (GEMGeometry_->regions()[0]->stations().size() > 1 and not (GEMGeometry_->regions()[0]->stations()[1]->superChambers().empty()))
  {
    npadsGE21  = GEMGeometry_->regions()[0]->stations()[1]->superChambers()[0]->chambers()[0]->etaPartitions()[0]->npads();
  }

  for(auto& region : GEMGeometry_->regions())
  {
    int region_id = region->region();
    unsigned ridx = getRegionIndex(region_id);

    if( auto* tmp_zr = bookZROccupancy(ibooker, "copad", "CoPad", region_id))
      me_occ_zr_[ridx] = tmp_zr;
    else
      edm::LogError("GEMCoPadDigiValidation") << "Failed to create simpleZR histograms" << std::endl;

    for( auto& station : region->stations())
    {
      int station_id = station->station();
      unsigned sidx = getStationIndex(station_id);

      if(auto* tmp_det = bookDetectorOccupancy(ibooker, station, "copad", "CoPad", region_id))
        me_occ_det_[ridx][sidx] = tmp_det;
      else
        edm::LogError("GEMCoPadDigiValidation") << "Failed to create DCEta histograms" << std::endl;

    } // station loop end
  } // region loop end


  if ( detailPlot_)
  {
    for(auto&  region : GEMGeometry_->regions())
    {
      int region_id = region->region();
      int ridx = (region_id + 1) / 2;

      for( auto& station : region->stations() )
      {
        int station_id = station->station();
        int sidx = station_id - 1;
          
        int nPads = station_id == 1 ? npadsGE11 : npadsGE21;

        me_detail_occ_zr_[ridx][sidx] = bookZROccupancy(ibooker, "copad_dg","CoPad Digi", region_id, station_id);
        me_detail_occ_xy_[ridx][sidx] = bookXYOccupancy(ibooker, "copad_dg","CoPad Digi", region_id, station_id);

        const char* name_suffix  = GEMUtils::getSuffixName(region_id, station_id).Data();
        const char* title_suffix = GEMUtils::getSuffixTitle(region_id, station_id).Data();

        TString name, title;

        name = TString::Format("copad_digi_occ_phi_pad%s", name_suffix); 
        title = TString::Format("Digi occupancy:%s; phi [rad]; Pad number", title_suffix);
        me_detail_occ_phi_pad_[ridx][sidx] = ibooker.book2D(name, title, 280, -PI, PI, nPads/2, 0, nPads);

        name = TString::Format("copad_dg_occ_pad%s", name_suffix);
        title = TString::Format("Digi occupancy per pad number: %s;Pad number; entries", title_suffix);
        me_detail_occ_pad_[ridx][sidx] = ibooker.book1D(name, title, nPads, 0.5, nPads+0.5);

        name = TString::Format("copad_dg_bx%s", name_suffix);
        title = TString::Format("Bunch crossing: %s; bunch crossing ; entries", title_suffix);
        me_detail_bx_[ridx][sidx] = ibooker.book1D(name ,title, 11, -5.5,5.5);

        name = TString::Format("copad_dg_xy%s_even",name_suffix);
        title = TString::Format("Digi XY occupancy %s at even chambers",title_suffix);
        me_detail_occ_xy_chamber_[ridx][sidx][0] = ibooker.book2D(name, title, 360, -360,360, 360, -360, 360);

        name = TString::Format("copad_dg_xy%s_odd", name_suffix);
        title = TString::Format("Digi XY occupancy %s at odd chambers",title_suffix);
        me_detail_occ_xy_chamber_[ridx][sidx][1] = ibooker.book2D(name, title, 360, -360,360, 360, -360, 360);

      } // station for loop
    } // region for loop
  } // detailPlot if statement
}


GEMCoPadDigiValidation::~GEMCoPadDigiValidation() {


}


void GEMCoPadDigiValidation::analyze(const edm::Event& e,
                                     const edm::EventSetup& iSetup)
{
  const GEMGeometry* GEMGeometry_ = initGeometry(iSetup);

  edm::Handle<GEMCoPadDigiCollection> gem_digis;
  e.getByToken(InputTagToken_, gem_digis);

  if (not gem_digis.isValid())
  {
    edm::LogError("GEMCoPadDigiValidation") << "Cannot get pads by token.";
    return ;
  }

  for (GEMCoPadDigiCollection::DigiRangeIterator cItr=gem_digis->begin(); cItr!=gem_digis->end(); cItr++)
  {
    GEMDetId id = (*cItr).first;
    int region_id = id.region();
    int station_id = id.station();
    int layer_id = id.layer();
    Short_t chamber = (Short_t) id.chamber();


    //loop over digis of given roll
    GEMCoPadDigiCollection::const_iterator digiItr;
    for (digiItr = (*cItr ).second.first; digiItr != (*cItr ).second.second; ++digiItr)
    {
      // GEM copads are stored per super chamber!
      GEMDetId schId = GEMDetId(region_id, id.ring(), station_id, 0, chamber, 0);
      Short_t nroll = (*digiItr).roll();
      LogDebug("GEMCoPadDigiValidation")<<"schId : "<<schId;
      const GeomDet* gdet = GEMGeometry_->idToDet(schId);
      if ( gdet == nullptr)
      {
        edm::LogError("GEMCoPadDigiValidation")<<schId<<" : This detId cannot be loaded from GEMGeometry // Original"<<id<<" station : "<<station_id;
        edm::LogError("GEMCoPadDigiValidation")<<"Getting DetId failed. Discard this gem copad hit. ";
        continue; 
      }
      const BoundPlane & surface = gdet->surface();
      const GEMSuperChamber * superChamber = GEMGeometry_->superChamber(schId);
      LogDebug("GEMCoPadDigiValidation")<<" #pads in this partition : "<< superChamber->chamber(1)->etaPartition(1)->npads();

      Short_t pad1 = (Short_t) digiItr->pad(1);
      Short_t pad2 = (Short_t) digiItr->pad(2);
      Short_t bx1  = (Short_t) digiItr->bx(1);
      Short_t bx2  = (Short_t) digiItr->bx(2);
      LogDebug("GEMCoPadDigiValidation")<<" copad #1 pad : "<<pad1<<"  bx : "<<bx1;
      LogDebug("GEMCoPadDigiValidation")<<" copad #2 pad : "<<pad2<<"  bx : "<<bx2;

      // Filtered using BX
      if ( bx1 < (Short_t)minBXGEM_ or bx1 > (Short_t)maxBXGEM_) continue;
      if ( bx2 < (Short_t)minBXGEM_ or bx2 > (Short_t)maxBXGEM_) continue;

      LocalPoint lp1 = superChamber->chamber(1)->etaPartition(nroll)->centreOfPad(pad1);
      LocalPoint lp2 = superChamber->chamber(2)->etaPartition(nroll)->centreOfPad(pad2);

      GlobalPoint gp1 = surface.toGlobal(lp1);
      GlobalPoint gp2 = surface.toGlobal(lp2);
      Float_t g_r1 = (Float_t) gp1.perp();
      Float_t g_r2 = (Float_t) gp2.perp();
      Float_t g_z1 = (Float_t) gp1.z();
      Float_t g_z2 = (Float_t) gp2.z();

      Float_t g_phi = (Float_t) gp1.phi();
      Float_t g_x = (Float_t) gp1.x();
      Float_t g_y = (Float_t) gp1.y();

      int ridx=0;
      if ( region_id == -1 ) ridx = 0 ; 
      else if (region_id == 1 ) ridx = 1; 
      else {
        edm::LogError("GEMCoPadDigiValidation")<<"region : "<<region_id<<std::endl;
      }
      int binX = (chamber-1)*2+layer_id;
      int binY = nroll;
      int sidx = station_id-1;

      // Fill normal plots.
      me_occ_zr_[ridx]->Fill( fabs(g_z1), g_r1);
      me_occ_zr_[ridx]->Fill( fabs(g_z2), g_r2);

      me_occ_det_[ridx][sidx]->Fill( binX, binY);
      me_occ_det_[ridx][sidx]->Fill( binX + 1, binY);

      // Fill detail plots.
      if ( detailPlot_)
      {
        me_detail_occ_xy_[ridx][sidx]->Fill(g_x,g_y);     
        me_detail_occ_phi_pad_[ridx][sidx]->Fill(g_phi,pad1);
        me_detail_occ_pad_[ridx][sidx]->Fill(pad1);
        me_detail_bx_[ridx][sidx]->Fill(bx1);
        me_detail_bx_[ridx][sidx]->Fill(bx2);
        me_detail_occ_zr_[ridx][sidx]->Fill(g_z1,g_r1);
        me_detail_occ_zr_[ridx][sidx]->Fill(g_z2,g_r2);

        me_detail_occ_xy_chamber_[ridx][sidx][static_cast<unsigned>(chamber%2)]->Fill(g_x,g_y);
      }
    }
  }
}
