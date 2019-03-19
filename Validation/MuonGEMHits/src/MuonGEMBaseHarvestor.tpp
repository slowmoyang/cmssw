#include "Validation/MuonGEMHits/interface/MuonGEMBaseHarvestor.h"


template <typename T>
T* MuonGEMBaseHarvestor::getElement(DQMStore::IGetter & getter,
                                    const TString & path) {
  std::string folder = gSystem->DirName(path);
  std::string name = gSystem->BaseName(path);

  getter.setCurrentFolder(folder);
  std::vector<std::string> mes = getter.getMEs();

  Bool_t not_found = std::find(mes.begin(), mes.end(), name) == mes.end();
  if (not_found) {
    edm::LogInfo(log_category_) << "doesn't contain " << path << std::endl;
    return nullptr;
  }

  T* hist = nullptr;
  if (auto tmp_me = getter.get(path.Data())) {
    hist = dynamic_cast<T*>(tmp_me->getRootObject()->Clone());
    hist->Sumw2();
  } else {
    edm::LogInfo(log_category_) << "failed to get " << path << std::endl;
  }

  return hist;
}
