#include "Validation/MuonGEMDigis/plugins/MuonGEMDigisHarvestor.h"

#include "Validation/MuonGEMHits/interface/GEMValidationUtils.h"

#include "FWCore/Framework/interface/MakerMacros.h"

#include "TSystem.h"
#include "TString.h"

#include <string>
#include <vector>


MuonGEMDigisHarvestor::MuonGEMDigisHarvestor(const edm::ParameterSet& ps)
    : MuonGEMBaseHarvestor(ps) {
  strip_folder_ = ps.getUntrackedParameter<std::string>("stripFolder");
  pad_folder_ = ps.getUntrackedParameter<std::string>("padFolder");
  copad_folder_ = ps.getUntrackedParameter<std::string>("copadFolder");

  region_ids_ = ps.getUntrackedParameter< std::vector<Int_t> >("regionIds");
  station_ids_ = ps.getUntrackedParameter< std::vector<Int_t> >("stationIds");
  layer_ids_ = ps.getUntrackedParameter< std::vector<Int_t> >("layerIds");
}


MuonGEMDigisHarvestor::~MuonGEMDigisHarvestor() {

}


void MuonGEMDigisHarvestor::dqmEndJob(DQMStore::IBooker & ibooker,
                                      DQMStore::IGetter & igetter) {

  // TODO harvsetStripDigi();
  igetter.setCurrentFolder(strip_folder_);

  for (Int_t region_id : region_ids_) {
    TString name_suffix_re = GEMUtils::getSuffixName(region_id);
    TString title_suffix_re = GEMUtils::getSuffixTitle(region_id);

    bookEff1D(ibooker, igetter, strip_folder_,
              "matched_strip_occ_eta" + name_suffix_re,
              "muon_simhit_occ_eta" + name_suffix_re,
              "eff_eta" + name_suffix_re,
              "Eta Efficiency (Muon Only)" + title_suffix_re);
              
    for (Int_t station_id : station_ids_) {
      TString name_suffix_re_st = GEMUtils::getSuffixName(region_id, station_id);
      TString title_suffix_re_st = GEMUtils::getSuffixTitle(region_id, station_id);

      bookEff1D(ibooker, igetter, strip_folder_,
                "matched_strip_occ_phi" + name_suffix_re_st,
                "muon_simhit_occ_phi" + name_suffix_re_st,
                "eff_phi" + name_suffix_re_st,
                "Phi Efficiency (Muon Only)" + title_suffix_re_st);

      bookEff2D(ibooker, igetter, strip_folder_,
                "matched_strip_occ_det" + name_suffix_re_st,
                "muon_simhit_occ_det" + name_suffix_re_st,
                "eff_det" + name_suffix_re_st,
                "Detector Component Efficiency (Muon Only)" + title_suffix_re_st);

    } // end loop over station ids
  } // end loop over region ids
}


//define this as a plug-in
DEFINE_FWK_MODULE(MuonGEMDigisHarvestor);
