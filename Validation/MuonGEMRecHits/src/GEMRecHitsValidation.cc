#include "Validation/MuonGEMRecHits/interface/GEMRecHitsValidation.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include <iomanip>

using namespace std;


GEMRecHitsValidation::GEMRecHitsValidation(const edm::ParameterSet& cfg): GEMBaseValidation(cfg) {
  InputTagToken_   = consumes<edm::PSimHitContainer>(cfg.getParameter<edm::InputTag>("simInputLabel"));
  InputTagToken_RH = consumes<GEMRecHitCollection>(cfg.getParameter<edm::InputTag>("recHitsInputLabel"));
  detailPlot_ = cfg.getParameter<bool>("detailPlot");
  folder_ = cfg.getParameter<std::string>("folder");
}


GEMRecHitsValidation::~GEMRecHitsValidation() {
}


void GEMRecHitsValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                          edm::Run const & Run,
                                          edm::EventSetup const & iSetup)
{
  const GEMGeometry* kGEMGeometry = initGeometry(iSetup);
  if ( kGEMGeometry == nullptr) {
    edm::LogError("GEMRecHitsValidation") << "Failed to initialise GEMGeometry in the bookHistograms step." << std::endl;
    return ;  
  }

  ibooker.setCurrentFolder(folder_);


  // Book histograms
  for(auto & region : kGEMGeometry->regions()) {
    int region_id = region->region();
    unsigned ridx = getRegionIndex(region_id);

    if(auto* tmp_zr = bookZROccupancy(ibooker, "rechit", "RecHit", region_id)) 
      me_occ_zr_[ridx] = tmp_zr;
    else
      edm::LogError("GEMRecHitsValidation") << "Failed to create simpleZR histograms" << std::endl;

    for(auto& station : region->stations()) {
      int station_id = station->station();
      unsigned sidx = getStationIndex(station_id);

      if (auto* tmp_det = bookDetectorOccupancy(ibooker, station, "rechit", "RecHit", region_id))
        me_occ_det_[ridx][sidx] = tmp_det;
      else
        edm::LogError("GEMRecHitsValidation") << "Failed to create occupancy per detector component" << std::endl;

    } // station loop end

    me_pull_x_[ridx] = bookHist1D(ibooker, "pullX", "Pull Of X", 100, -50, 50, region_id);
    me_pull_y_[ridx] = bookHist1D(ibooker, "pullY", "Pull Of Y", 100, -50, 50, region_id);

  } // region loop end

  me_cls_ = ibooker.book1D("gem_cls_tot", "ClusterSize Distribution", 11, -0.5, 10.5);
  
  if(detailPlot_) {
    // TODO Debug

    for(const auto & region : kGEMGeometry->regions()) {
      int region_id = region->region();
      unsigned ridx = getRegionIndex(region_id);

      for(const auto& station : region->stations()) {
        int station_id = station->station();
        unsigned sidx = getStationIndex(station_id);

        // TODO
        for(int layer_id : {1, 2}) {
          unsigned lidx = getLayerIndex(layer_id);

          // bookHist1D(ibooker, name, title, nbinsx, xlow, xup, region_id, station_id, layer_id)
          me_detail_cls_[ridx][sidx][lidx] = bookHist1D(
              ibooker, "cls", "ClusterSize Distribution",
              11, -0.5, 10.5,
              region_id, station_id, layer_id);

          me_detail_pull_x_[ridx][sidx][lidx] = bookHist1D(
              ibooker, "pull_x", "Pull Of X",
              100, -50, 50,
              region_id, station_id, layer_id);

          me_detail_pull_y_[ridx][sidx][lidx] = bookHist1D(
              ibooker, "pull_y", "Pull Of Y",
              100, -50, 50,
              region_id, station_id, layer_id);

          me_detail_occ_zr_[ridx][sidx][lidx] = bookZROccupancy(
              ibooker, "rechit", "RecHits", region_id, station_id, layer_id);

          me_detail_occ_xy_[ridx][sidx][lidx] = bookXYOccupancy(
              ibooker, "rechit", "RecHits", region_id, station_id, layer_id);

          me_detail_occ_polar_[ridx][sidx][lidx] = bookPolarOccupancy(
              ibooker, "rechit", "recHits", region_id, station_id, layer_id);
                                                                      
          // bookHist1D(ibooker, name, title, region_id, station_id, layer_id, is_odd_chamber)
          me_detail_sim_occ_eta_[ridx][sidx][lidx][0] = bookHist1D(
              ibooker, "sh_occ_eta", "SimHit Eta Occupancy",
              51, -4, 4,
              region_id, station_id, layer_id, false);

          me_detail_rec_occ_eta_[ridx][sidx][lidx][1] = bookHist1D(
              ibooker, "rh_occ_eta", "RecHit Eta Occupancy",
              51, -4, 4,
              region_id, station_id, layer_id, true);

          me_detail_sim_occ_phi_[ridx][sidx][lidx][0] = bookHist1D(
              ibooker, "sh_occ_phi", "SimHit",
              51, -TMath::Pi(), TMath::Pi(),
              region_id, station_id, layer_id, false);

          me_detail_rec_occ_phi_[ridx][sidx][lidx][1] = bookHist1D(
              ibooker, "rh_occ_phi", "RecHit",
              51, -TMath::Pi(), TMath::Pi(),
              region_id, station_id, layer_id, true);
        } // layer loop
      } // station loop
    } // region loop
  } // detailPlot 


  LogDebug("GEMRecHitsValidation")<<"Booking End.\n";
}



void GEMRecHitsValidation::analyze(const edm::Event& e,
                                   const edm::EventSetup& iSetup) {
  const GEMGeometry* kGEMGeometry  = initGeometry(iSetup);
  if ( kGEMGeometry == nullptr) {
    // TODO LogError
    return; 
  }

  edm::Handle<GEMRecHitCollection> rechit_container;
  edm::Handle<edm::PSimHitContainer> simhit_container;
  e.getByToken(InputTagToken_, simhit_container);
  e.getByToken(InputTagToken_RH, rechit_container);


  if (not simhit_container.isValid()) {
    edm::LogError("GEMRecHitsValidation") << "Failed to get PSimHitContainer.\n";
    return ;
  }

  if (not rechit_container.isValid())
  {
    edm::LogError("GEMRecHitsValidation") << "Failed to get GEMRecHitCollection\n";
    return ;
  }


  for(const auto & simhit : *simhit_container.product()) {
    /* TODO
     * in the GEMBaseValidation level
     * if(not isGoodSimHit(simhit))
     * {
     *   LogError
     *   continue
     * }
     *
     */
    if (not (std::abs(simhit.particleType()) == kMuonPDGId_)) {
      // FIXME LogError
      edm::LogError("GEMRecHitsValidation") << "PSimHit is not muon\n";
      continue;
    }

    if (not kGEMGeometry->idToDet(simhit.detUnitId())) {
      // FIXME LogError
      std::cout << "simHit did not matched with GEMGeometry." << std::endl;
      continue;
    }

    const unsigned kSimDetUnitId = simhit.detUnitId();
    GEMDetId sim_id(kSimDetUnitId);

    int region_id = sim_id.region();
    int station_id = sim_id.station();
    int layer_id = sim_id.layer();

    int chamber_id = sim_id.chamber();
    int roll_id = sim_id.roll();

    unsigned ridx = getRegionIndex(region_id);
    unsigned sidx = getStationIndex(station_id);
    unsigned lidx = getLayerIndex(layer_id);
    // FIXME
    unsigned cidx = chamber_id % 2;


    LocalPoint sim_local = simhit.localPosition();
    GlobalPoint sim_global = kGEMGeometry->idToDet(sim_id)->surface().toGlobal(sim_local);

    int sim_fired_strip = kGEMGeometry->etaPartition(kSimDetUnitId)->strip(sim_local);

    if(detailPlot_) {
      me_detail_sim_occ_eta_[ridx][sidx][lidx][cidx]->Fill(sim_global.eta());
      me_detail_sim_occ_phi_[ridx][sidx][lidx][cidx]->Fill(sim_global.phi());
    }


    GEMRecHitCollection::range range = rechit_container->get(sim_id);
    // for(GEMRecHitCollection::const_iterator rechit = range.first; rechit != range.second; ++rechit) {
    for(auto rechit = range.first; rechit != range.second; ++rechit) {
      Int_t cls = rechit->clusterSize();

      // matching conditions
      bool matched;
      if ( cls == 1 ) {
        matched = sim_fired_strip == rechit->firstClusterStrip();
      }
      else {
        Int_t rec_first_fired_strip = rechit->firstClusterStrip();
        Int_t rec_last_fired_strip = rec_first_fired_strip + cls - 1;
        matched = (sim_fired_strip >= rec_first_fired_strip) and (sim_fired_strip <= rec_last_fired_strip);
      }

      if(matched) {
        LocalPoint rec_local = rechit->localPosition();
        GlobalPoint rec_global = kGEMGeometry->idToDet(kSimDetUnitId)->surface().toGlobal(rec_local);

        // XXX
        // 
        Float_t  resolution_x = std::sqrt(rechit->localPositionError().xx());
        Float_t  resolution_y = std::sqrt(rechit->localPositionError().yy());

        Float_t residual_x = rec_local.x() - sim_local.x();
        Float_t residual_y = rec_local.y() - sim_local.y(); 
        Float_t pull_x = residual_x / resolution_x;
        Float_t pull_y = residual_y / resolution_y;

        // DataFormats/GeometryVector/interface/extBasic3DVector.h
        // ::perp() Magnitude of transverse component 
        me_occ_zr_[ridx]->Fill(std::fabs(rec_global.z()), rec_global.perp());

        int bin_x = (chamber_id-1) * 2 + lidx;
        me_occ_det_[ridx][sidx]->Fill(bin_x, roll_id);

        me_cls_->Fill(cls);
        me_pull_x_[ridx]->Fill(pull_x);
        me_pull_y_[ridx]->Fill(pull_y);

        if(detailPlot_) {
          me_detail_cls_[ridx][sidx][lidx]->Fill(cls);
          me_detail_pull_x_[ridx][sidx][lidx]->Fill(pull_x);
          me_detail_pull_y_[ridx][sidx][lidx]->Fill(pull_y);
          me_detail_occ_zr_[ridx][sidx][lidx]->Fill(rec_global.z(), rec_global.perp());
          me_detail_occ_xy_[ridx][sidx][lidx]->Fill(rec_global.x(), rec_global.y());
          me_detail_occ_polar_[ridx][sidx][lidx]->Fill(rec_global.phi().phi(), rec_global.perp());

          me_detail_rec_occ_eta_[ridx][sidx][lidx][cidx]->Fill(rec_global.eta());
          me_detail_rec_occ_phi_[ridx][sidx][lidx][cidx]->Fill(rec_global.phi());
        }

        break;

      }
    } // RecHits Loop End
  } // SimHit Loop End

}
