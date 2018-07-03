#include "Validation/MuonGEMHits/interface/GEMHitsValidation.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"

#include "DataFormats/Common/interface/Handle.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"

#include <exception>

using namespace std;

GEMHitsValidation::GEMHitsValidation(const edm::ParameterSet& cfg) : GEMBaseValidation(cfg) {
  InputTagToken_ = consumes<edm::PSimHitContainer>(cfg.getParameter<edm::InputTag>("simInputLabel"));

  TOFRange_ = cfg.getUntrackedParameter< std::vector<double> >("TOFRange");
  detailPlot_ = cfg.getParameter<bool>("detailPlot");
}


GEMHitsValidation::~GEMHitsValidation() {
}


void GEMHitsValidation::bookHistograms(DQMStore::IBooker & ibooker,
                                       edm::Run const & Run,
                                       edm::EventSetup const & iSetup) {
  const GEMGeometry* kGEMGeometry = initGeometry(iSetup);
  if ( kGEMGeometry == nullptr) {
    edm::LogError("GEMHitsValidation") << "Cannot initialise GEMGeometry in the bookHistograms step.\n";
    return;
  }

  ibooker.setCurrentFolder("MuonGEMHitsV/GEMHitsTask");

  LogDebug("MuonGEMHitsValidation") << "+++ Region independant part.\n";
  for(auto& station : kGEMGeometry->regions()[0]->stations()) {
    int station_id = station->station();
    unsigned sidx = getStationIndex(station_id);

    double tof_min, tof_max;
    std::tie(tof_min, tof_max) = getTOFRange(station_id);

    const char* name_tof  = TString::Format("tof_mu_st%d", station_id).Data();
    const char* title_tof = TString::Format("SimHit TOF (Muon only) : Station %d ; Time of flight [ns] ; entries", station_id).Data();
    me_tof_mu_[sidx] = ibooker.book1D(name_tof, title_tof, 40, tof_min, tof_max);

    const char* name_eloss  = TString::Format("eloss_mu_s%d", station_id).Data();
    const char* title_eloss = TString::Format("SimHit Energy Loss (Muon only) : Station %d ; Energy loss [eV] ; entries", station_id).Data();
    me_eloss_mu_[sidx] = ibooker.book1D(name_eloss, title_eloss, 60, 0.0, 6000.0);
  } // STATION LOOP END


  LogDebug("MuonGEMHitsValidation") << "+++ Region+Station part.\n";
  // Regions, Region+station
  for(auto& region : kGEMGeometry->regions()) {
    int region_id = region->region();
    unsigned ridx = getRegionIndex(region_id);

    LogDebug("MuonGEMHitsValidation") << "+++ SimpleZR Occupancy\n";
    if(MonitorElement* simpleZR = bookZROccupancy(ibooker, "simhit", "SimHit", region_id)) {
      me_occ_zr_[ridx] = simpleZR;
    }
    else {
      edm::LogError("GEMHitsValidation") << "Cannot book ZR \n";
    }

    for(auto& station : region->stations()) {
      int station_id = station->station();
      unsigned sidx = getStationIndex(station_id);

      LogDebug("MuonGEMHitsValidation") << "+++ dcEta Occupancy\n";

      if(MonitorElement* det_occ_tmp = bookDetectorOccupancy(ibooker, station, "sh", "SimHit", region_id)) {
        me_occ_det_[ridx][sidx] = det_occ_tmp;
      }
      else {
        edm::LogError("GEMHitsValidation") << "Cannot book " << std::endl
                                           << "GEMHits Detector Occ "
                                           << region_id;
      }

    } // STATION LOOP END
  } // REGION LOOP END


  LogDebug("MuonGEMHitsValidation") << "+++ Begining Detail Plots\n";
  if(detailPlot_ )
  {
    for(auto& region : kGEMGeometry->regions())
    {
      int region_id = region->region();
      unsigned ridx = getRegionIndex(region_id);

      for(auto& station : region->stations())
      {
        int station_id = station->station();
        unsigned sidx = getStationIndex(station_id);

        double tof_min, tof_max;
        std::tie(tof_min, tof_max) = getTOFRange(station_id);

        for(int layer_id : {1, 2})
        {
          unsigned lidx = getLayerIndex(layer_id);

          me_detail_occ_zr_[ridx][sidx][lidx] = bookZROccupancy(ibooker, "simhit", "SimHit", region_id, station_id, layer_id);
          me_detail_occ_xy_[ridx][sidx][lidx] = bookXYOccupancy(ibooker, "simhit", "SimHit", region_id, station_id, layer_id);

          const char* name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id);
          const char* title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id); 

          TString name, title;

          name = TString::Format("tof%s", name_suffix);
          title = TString::Format("SimHit TOF : %s; Time of flight [ns] ; entries", title_suffix);
          me_detail_tof_[ridx][sidx][lidx] = ibooker.book1D(name, title, 40, tof_min, tof_max);

          name    = TString::Format("tof_muon%s", name_suffix);
          title   = TString::Format("SimHit TOF(Muon only) : %s; Time of flight [ns] ; entries", title_suffix);
          me_detail_tof_mu_[ridx][sidx][lidx] = ibooker.book1D(name, title, 40, tof_min, tof_max);

          name     = TString::Format("eloss%s", name_suffix);
          title    = TString::Format("SimHit Energy loss : %s; Energy loss [eV] ; entries", title_suffix);
          me_detail_eloss_[ridx][sidx][lidx] = ibooker.book1D(name, title, 60, 0, 6000);

          name  = TString::Format("eloss_muon%s", name_suffix);
          title = TString::Format("SimHit Energy loss(Muon only) : %s; Energy loss [eV] ; entries", title_suffix);
          me_detail_eloss_mu_[ridx][sidx][lidx] = ibooker.book1D(name, title, 60,0.,6000.);

        } // LAYER LOOP END

        const char* name_suffix = GEMUtils::getSuffixName(region_id, station_id);
        const char* title_suffix= GEMUtils::getSuffixTitle(region_id, station_id);
          
        TString hist_name = TString::Format("me_xy_occ_%s", name_suffix);
        TString hist_title = TString::Format("Simhit Global XY Plots at %s", title_suffix);

        me_detail_occ_xy_chamber_[ridx][sidx][0] = ibooker.book2D(hist_name+"_even", (hist_title +" even").Data(),nBinXY_,-360,360,nBinXY_,-360,360);
        me_detail_occ_xy_chamber_[ridx][sidx][1] = ibooker.book2D(hist_name+"_odd", (hist_title +" odd").Data(),nBinXY_,-360,360,nBinXY_,-360,360);


      } // STATION LOOP END
    } // REGION LOOP END
  } // detailPlot IF END



  me_gem_eta_phi_ = ibooker.book2D("eta_phi", "eta_phi", 101, -4, 4, 101, -3.14, 3.14);
  for(const auto & det_id : kGEMGeometry->detUnitIds())
  {
    GEMDetId gem_id(det_id);

    const GEMEtaPartition* eta_partition = kGEMGeometry->etaPartition(gem_id);
    LocalPoint local_origin(0.0, 0.0, 0.0);
    GlobalPoint gp = eta_partition->toGlobal(local_origin);
    auto eta = gp.eta();
    auto phi = gp.phi();
    me_gem_eta_phi_->Fill(eta, phi);
  }


  LogDebug("GEMHitsValidation") << "Booking End." << std::endl;

}


std::tuple<double, double> GEMHitsValidation::getTOFRange(int station_id)
{
  unsigned start_index = station_id == 1 ? 0 : 2;
  double tof_min = TOFRange_[start_index];
  double tof_max = TOFRange_[start_index + 1];
  return std::make_tuple(tof_min, tof_max);
}





void GEMHitsValidation::analyze(const edm::Event& e,
                                const edm::EventSetup& iSetup) {
  const GEMGeometry* kGEMGeometry = initGeometry(iSetup) ;

  edm::Handle<edm::PSimHitContainer> simhit_container;
  e.getByToken(InputTagToken_, simhit_container);

  if (not simhit_container.isValid()) {
    edm::LogError("GEMHitsValidation") << "Cannot get GEMHits by Token simInputTagToken";
    return ;
  }

  for(const auto & simhit : *simhit_container.product()) {

    if ( kGEMGeometry->idToDet(simhit.detUnitId()) == nullptr) {
      std::cout<<"simHit did not matched with GEMGeometry."<<std::endl;
      continue;
    }

    const GEMDetId kId(simhit.detUnitId());

    Int_t region_id = kId.region();
    Int_t station_id =  kId.station();
    Int_t layer_id = kId.layer();
    Int_t chamber_id =  kId.chamber();
    Int_t roll_id =  kId.roll(); // eta partition

    unsigned ridx = getRegionIndex(region_id);
    unsigned sidx = getStationIndex(station_id);
    unsigned lidx = getLayerIndex(layer_id);

    //const LocalPoint p0(0., 0., 0.);
    //const GlobalPoint Gp0(kGEMGeometry->idToDet(hits->detUnitId())->surface().toGlobal(p0));
    const LocalPoint hitLP(simhit.localPosition());
    const GlobalPoint hitGP(kGEMGeometry->idToDet(simhit.detUnitId())->surface().toGlobal(hitLP));

    Float_t g_r = hitGP.perp();
    Float_t g_x = hitGP.x();
    Float_t g_y = hitGP.y();
    Float_t g_z = hitGP.z();
    Float_t energyLoss = kEnergyCF_ * simhit.energyLoss();
    Float_t timeOfFlight = simhit.timeOfFlight();

    me_occ_zr_[ridx]->Fill(std::fabs(g_z), g_r);

    int binX = (chamber_id - 1) * 2 + lidx;
    int binY = roll_id;
    me_occ_det_[ridx][sidx]->Fill(binX, binY);

    if (std::abs(simhit.particleType()) == kMuonPDGId_) {
      me_tof_mu_[sidx]->Fill( timeOfFlight );
      me_eloss_mu_[sidx]->Fill(energyLoss);
    }

    if( detailPlot_ ) {
      // First, fill variable has no condition.
      me_detail_occ_xy_[ridx][sidx][lidx]->Fill(g_z, g_r);
      me_detail_occ_xy_[ridx][sidx][lidx]->Fill(g_x, g_y);

      me_detail_tof_[ridx][sidx][lidx]->Fill(timeOfFlight);
      me_detail_eloss_[ridx][sidx][lidx]->Fill(energyLoss);

      if (std::abs(simhit.particleType()) == kMuonPDGId_)
      {
        me_detail_tof_mu_[ridx][sidx][lidx]->Fill(timeOfFlight);
        me_detail_eloss_mu_[ridx][sidx][lidx]->Fill(energyLoss);
      }

      // chamber id even or odd index
      unsigned even_odd_index = static_cast<unsigned>(chamber_id % 2);
      me_detail_occ_xy_chamber_[ridx][sidx][even_odd_index]->Fill(g_x, g_y); 
    } // detailPlot

  } // simhit loop END
}

