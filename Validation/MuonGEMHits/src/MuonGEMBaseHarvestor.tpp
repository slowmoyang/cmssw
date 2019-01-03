#include "Validation/MuonGEMHits/interface/MuonGEMBaseHarvestor.h"


template <typename T>
T* MuonGEMBaseHarvestor::getElement(DQMStore::IGetter & igetter,
                                    const std::string & folder,
                                    const TString & name) {
  const std::string path = gSystem->ConcatFileName(folder.c_str(), name);

  if (not igetter.containsAnyMonitorable(path)) {
    edm::LogInfo(log_category_) << "doesn't contain " << path << std::endl;
    return nullptr;
  }

  T* hist = nullptr;
  if (auto tmp_me = igetter.getElement(path)) {
    hist = dynamic_cast<T*>(tmp_me->getRootObject()->Clone());
    hist->Sumw2();
  } else {
    edm::LogInfo(log_category_) << "failed to get " << path << std::endl;
  }

  return hist;
}
