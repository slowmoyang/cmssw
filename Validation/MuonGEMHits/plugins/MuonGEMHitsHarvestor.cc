#include "Validation/MuonGEMHits/plugins/MuonGEMHitsHarvestor.h"

#include "FWCore/Framework/interface/MakerMacros.h" // DEFINE_FWK_MODULE

#include <string>

MuonGEMHitsHarvestor::MuonGEMHitsHarvestor(const edm::ParameterSet & ps) :
    MuonGEMBaseHarvestor(ps) {
  folder_ = ps.getParameter<std::string>("folder");
}


MuonGEMHitsHarvestor::~MuonGEMHitsHarvestor() {
}


void MuonGEMHitsHarvestor::dqmEndJob(DQMStore::IBooker & ibooker,
                                     DQMStore::IGetter & igetter) {
  igetter.setCurrentFolder(folder_);
}


//define this as a plug-in
DEFINE_FWK_MODULE(MuonGEMHitsHarvestor);
