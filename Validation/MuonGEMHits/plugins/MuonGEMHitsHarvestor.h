#ifndef Validation_MuonGEMHits_MuonGEMHitsHarvestor_H
#define Validation_MuonGEMHits_MuonGEMHitsHarvestor_H

#include "Validation/MuonGEMHits/interface/MuonGEMBaseHarvestor.h"

class MuonGEMHitsHarvestor : public MuonGEMBaseHarvestor {
public:
  /// constructor
  explicit MuonGEMHitsHarvestor(const edm::ParameterSet&);
  /// destructor
  ~MuonGEMHitsHarvestor() override;

  void dqmEndJob(DQMStore::IBooker&, DQMStore::IGetter&) override;
  void ProcessBooking(
      DQMStore::IBooker&, DQMStore::IGetter&, std::string label_suffix, TH1F* track_hist, TH1F* sh_hist = nullptr);
  TProfile* ComputeEff(TH1F* num, TH1F* denum);

private:
  std::string dbe_path_, outputFile_;
};
#endif
