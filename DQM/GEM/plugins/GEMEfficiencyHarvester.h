#ifndef DQM_GEM_GEMEfficiencyHarvester_h
#define DQM_GEM_GEMEfficiencyHarvester_h

/** \class GEMEfficiencyAnalyzer
 * 
 * DQM monitoring client for GEM efficiency and resolution
 * based on Validation/MuonGEMHits/MuonGEMBaseHarvestor
 *
 * \author Seungjin Yang <seungjin.yang@cern.ch>
 */

#include "DQMServices/Core/interface/DQMEDHarvester.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"

#include "DQM/GEM/interface/GEMDQMEfficiencyCalculator.h"


class GEMEfficiencyHarvester : public DQMEDHarvester {
public:
  GEMEfficiencyHarvester(const edm::ParameterSet&);
  ~GEMEfficiencyHarvester() override;
  static void fillDescriptions(edm::ConfigurationDescriptions&);
  void dqmEndJob(DQMStore::IBooker&, DQMStore::IGetter&) override;

private:
  void doEfficiency(DQMStore::IBooker&, DQMStore::IGetter&);

  std::vector<std::string> splitString(std::string, const std::string);
  std::tuple<std::string, int, int> parseResidualName(std::string, const std::string);
  void doResolution(DQMStore::IBooker&, DQMStore::IGetter&, const std::string);

  template <typename T>
  int findResolutionBin(const T&, const std::vector<T>&);

  const std::string kFolder_;
  const std::string kLogCategory_;
  const std::vector<std::string> kResolutionSummaryPrefixes_;

  std::unique_ptr<GEMDQMEfficiencyCalculator> efficiency_calculator_;
};

template <typename T>
int GEMEfficiencyHarvester::findResolutionBin(const T& elem, const std::vector<T>& vec) {
  auto iter = std::find(vec.begin(), vec.end(), elem);
  int bin = (iter != vec.end()) ? std::distance(vec.begin(), iter) + 1 : -1;
  return bin;
}

#endif  // DQM_GEM_GEMEfficiencyHarvester_h
