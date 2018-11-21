#include "Validation/MuonGEMRecHits/plugins/MuonGEMRecHitsHarvestor.h"
#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TEfficiency.h"


MuonGEMRecHitsHarvestor::MuonGEMRecHitsHarvestor(const edm::ParameterSet& ps)
    : MuonGEMBaseHarvestor(ps) {

  folder_ = ps.getParameter<std::string>("folder");

  region_ids_ = ps.getUntrackedParameter< std::vector<Int_t> >("regionIds");
  station_ids_ = ps.getUntrackedParameter< std::vector<Int_t> >("stationIds");
  layer_ids_ = ps.getUntrackedParameter< std::vector<Int_t> >("layerIds");
}


MuonGEMRecHitsHarvestor::~MuonGEMRecHitsHarvestor() {
}


void MuonGEMRecHitsHarvestor::dqmEndJob(DQMStore::IBooker& ibooker,
                                        DQMStore::IGetter& igetter) {
  // igetter.setCurrentFolder(folder_);
  igetter.cd(folder_);

  for (const auto & region_id : region_ids_) {
    TString name_suffix_re = GEMUtils::getSuffixName(region_id);
    TString title_suffix_re = GEMUtils::getSuffixTitle(region_id);

    bookEff1D(ibooker, igetter, folder_,
              "rechit_occ_eta" + name_suffix_re,
              "muon_occ_eta" + name_suffix_re,
              "eff_eta" + name_suffix_re,
              "Eta Efficiency" + title_suffix_re);

    bookEff1D(ibooker, igetter, folder_,
              "tight_rechit_occ_eta" + name_suffix_re,
              "tight_muon_occ_eta" + name_suffix_re,
              "eff_eta_tight" + name_suffix_re,
              "Eta Efficiency (CutBasedIdTight)" + title_suffix_re);

    bookEff1D(ibooker, igetter, folder_,
              "loose_rechit_occ_eta" + name_suffix_re,
              "loose_muon_occ_eta" + name_suffix_re,
              "eff_eta_loose" + name_suffix_re,
              "Eta Efficiency (CutBasedIdLoose)" + title_suffix_re);

    for (const auto & station_id : station_ids_) {
      TString name_suffix_re_st = GEMUtils::getSuffixName(region_id, station_id);
      TString title_suffix_re_st = GEMUtils::getSuffixTitle(region_id, station_id);

      // NOTE Phi 
      bookEff1D(ibooker, igetter, folder_,
                "rechit_occ_phi" + name_suffix_re_st,
                "muon_occ_phi" + name_suffix_re_st,
                "eff_phi" + name_suffix_re_st,
                "Phi Efficiency" + title_suffix_re_st);

      bookEff1D(ibooker, igetter, folder_,
                "tight_rechit_occ_phi" + name_suffix_re_st,
                "tight_muon_occ_phi" + name_suffix_re_st,
                "eff_phi_tight" + name_suffix_re_st,
                "Phi Efficiency (CutBasedIdTight)" + title_suffix_re_st);

      bookEff1D(ibooker, igetter, folder_,
                "loose_rechit_occ_phi" + name_suffix_re_st,
                "loose_muon_occ_phi" + name_suffix_re_st,
                "eff_phi_loose" + name_suffix_re_st,
                "Phi Efficiency (CutBasedIdLoose)" + title_suffix_re_st);

      // NOTE Detector Component 
      bookEff2D(ibooker, igetter, folder_, 
                "rechit_occ_det" + name_suffix_re_st,
                "muon_occ_det" + name_suffix_re_st,
                "eff_det" + name_suffix_re_st,
                "Detector Component Efficiency" + title_suffix_re_st);

      bookEff2D(ibooker, igetter, folder_, 
                "tight_rechit_occ_det" + name_suffix_re_st,
                "tight_muon_occ_det" + name_suffix_re_st,
                "eff_det_tight" + name_suffix_re_st,
                "Detector Component Efficiency (CutBasedIdTight)" + title_suffix_re_st);

      bookEff2D(ibooker, igetter, folder_, 
                "loose_rechit_occ_det" + name_suffix_re_st,
                "loose_muon_occ_det" + name_suffix_re_st,
                "eff_det_loose" + name_suffix_re_st,
                "Detector Component Efficiency (CutBasedIdLoose)" + title_suffix_re_st);


    } // Station Id END
  } // Region Id END

}


//define this as a plug-in
DEFINE_FWK_MODULE(MuonGEMRecHitsHarvestor);

