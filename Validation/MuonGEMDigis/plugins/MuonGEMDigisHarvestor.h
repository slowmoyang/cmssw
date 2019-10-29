#ifndef Validation_MuonGEMDigis_MuonGEMDigisHarvestor_H
#define Validation_MuonGEMDigis_MuonGEMDigisHarvestor_H

#include "Validation/MuonGEMHits/interface/MuonGEMBaseHarvestor.h"

class MuonGEMDigisHarvestor : public MuonGEMBaseHarvestor {
public:
  /// constructor
  explicit MuonGEMDigisHarvestor(const edm::ParameterSet&);
  /// destructor
  ~MuonGEMDigisHarvestor() override;

  void dqmEndJob(DQMStore::IBooker&, DQMStore::IGetter&) override;
  void ProcessBooking(
      DQMStore::IBooker&, DQMStore::IGetter&, const char* label, TString suffix, TH1F* track_hist, TH1F* sh_hist);
  TProfile* ComputeEff(TH1F* num, TH1F* denum);

private:
  std::string dbe_path_, dbe_hist_prefix_, compareable_dbe_path_, compareable_dbe_hist_prefix_, outputFile_;
};
#endif
