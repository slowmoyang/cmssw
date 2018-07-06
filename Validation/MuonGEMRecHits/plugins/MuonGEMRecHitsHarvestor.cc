#include "Validation/MuonGEMRecHits/plugins/MuonGEMRecHitsHarvestor.h"

// system include files
#include <memory>


// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "TTree.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TH1F.h"
#include "TEfficiency.h"
#include "TProfile.h"
#include "TAxis.h"


///Data Format
#include <DataFormats/GEMRecHit/interface/GEMRecHit.h>
#include <DataFormats/GEMRecHit/interface/GEMRecHitCollection.h>
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "DataFormats/GeometrySurface/interface/LocalError.h"
#include "DataFormats/GeometryVector/interface/LocalPoint.h"
#include "DataFormats/Scalers/interface/DcsStatus.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Math/interface/deltaPhi.h"

#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"

///Geometry
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"

#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartition.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartitionSpecs.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"

#include "DQMServices/Core/interface/DQMStore.h"

///Log messages
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"



MuonGEMRecHitsHarvestor::MuonGEMRecHitsHarvestor(const edm::ParameterSet& ps) {
  dbe_path_ = "MuonGEMRecHitsV/GEMRecHitsTask/";

  region_ids_ = ps.getUntrackedParameter< std::vector<int> >("regionIds");
  station_ids_ = ps.getUntrackedParameter< std::vector<int> >("stationIds");
  layer_ids_ = ps.getUntrackedParameter< std::vector<int> >("layerIds");
}


MuonGEMRecHitsHarvestor::~MuonGEMRecHitsHarvestor()
{
}


TProfile* MuonGEMRecHitsHarvestor::computeEff(TH1F* passed, TH1F* total)
{
  std::string name  = "eff_" + std::string(passed->GetName());
  std::string title = "Eff. " + std::string(passed->GetTitle());
  TProfile * eff_profile = new TProfile(name.c_str(),
                                        title.c_str(),
                                        total->GetXaxis()->GetNbins(),
                                        total->GetXaxis()->GetXmin(),
                                        total->GetXaxis()->GetXmax());


  if(TEfficiency::CheckConsistency(*passed, *total))
  {
    TEfficiency        eff(*passed, *total);
    TGraphAsymmErrors* eff_graph = eff.CreateGraph();
    TAxis* x_axis = passed->GetXaxis();
    // TH1F* eff_hist = dynamic_cast<TH1F*>(eff_graph->GetHistogram()->Clone());

    // for (int bin = 1; bin <= eff_hist->GetNbinsX(); bin++)
    for(int i = 0; i < eff_graph->GetN(); ++i)
    {

      double x, y, error;
      bool get_point_fails = eff_graph->GetPoint(i, x, y) == -1;

      if(get_point_fails)
      {
        edm::LogError("MuonGEMRecHitsHarvestor") << "GetPoint failed" << std::endl;
        continue;
      }

      error = eff_graph->GetErrorYhigh(i);
      int bin = x_axis->FindBin(x);
      // double content = eff_hist->GetBinContent(bin);
      // double error = eff_hist->GetBinError(bin);
      eff_profile->SetBinContent(bin, y);
      eff_profile->SetBinError(bin, error);
      eff_profile->SetBinEntries(bin, 1);
    }
  }
  else
  {
    edm::LogError("MuonGEMRecHitsHarvestor") << "TEfficiency Inconsistency Error" << std::endl;
    std::cout << passed->GetName() << std::endl;
    std::cout << total->GetName() << std::endl;

    name += "_inconsistent";
    title += " inconsistent";
    eff_profile->SetName(name.c_str());
    eff_profile->SetTitle(title.c_str());
  }

  return eff_profile;
}


void MuonGEMRecHitsHarvestor::dqmEndJob(DQMStore::IBooker& ibooker,
                                        DQMStore::IGetter& ig) {
  ig.setCurrentFolder(dbe_path_.Data());

  for(int region_id : region_ids_) {
    for(int station_id : station_ids_) {
      for(int layer_id : layer_ids_) {
        for(bool is_odd_chamber: {false, true}) {
          for(const char* axis : {"eta", "phi"}) {

            const char* name_suffix = GEMUtils::getSuffixName(region_id, station_id, layer_id, is_odd_chamber);
            const char* title_suffix = GEMUtils::getSuffixTitle(region_id, station_id, layer_id, is_odd_chamber);

            const char* sim_name = TString::Format("sim_occ_%s%s", axis, name_suffix);
            const char* rec_name = TString::Format("rec_occ_%s%s", axis, name_suffix);

            TH1F* sim_occupancy;
            if(MonitorElement* tmp_sim = ig.get(sim_name)) {
              sim_occupancy = dynamic_cast<TH1F*>(tmp_sim->getTH1F()->Clone());
            } else {
              edm::LogError("MuonGEMRecHitsHarvestor") << "failed to get "
                                                       << sim_name
                                                       << std::endl;
              continue;
            }
            sim_occupancy->Sumw2(); // XXX Reason?

            TH1F* rec_occupancy;
            if(MonitorElement* tmp_rec = ig.get(rec_name)) {
              rec_occupancy = dynamic_cast<TH1F*>(tmp_rec->getTH1F()->Clone());
            } else {
              edm::LogError("MuonGEMRecHitsHarvestor") << "failed to get "
                                                       << rec_name
                                                       << std::endl;
              continue;
            }
            rec_occupancy->Sumw2();

            TProfile* eff = computeEff(rec_occupancy, sim_occupancy);

            TString title = TString::Format("Efficiency %s :%s", axis, title_suffix);
            TString x_title = TString::Format("#%s", axis);

            eff->SetTitle(title);
            eff->SetXTitle(x_title);

            TString name = TString::Format("eff_%s%s", axis, name_suffix);

            ibooker.bookProfile(name, eff);
          } // axis loop
        } // is_odd_chamber loop
      } // Layer Id END
    } // Station Id END
  } // Region Id END
}


//define this as a plug-in
DEFINE_FWK_MODULE(MuonGEMRecHitsHarvestor);

