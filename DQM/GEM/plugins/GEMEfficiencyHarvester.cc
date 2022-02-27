#include "DQM/GEM/plugins/GEMEfficiencyHarvester.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/isFinite.h"

#include "TEfficiency.h"
#include "TPRegexp.h"

GEMEfficiencyHarvester::GEMEfficiencyHarvester(const edm::ParameterSet& ps)
    : kFolder_(ps.getUntrackedParameter<std::string>("folder")),
      kLogCategory_(ps.getUntrackedParameter<std::string>("logCategory")),
      kResolutionSummaryPrefixes_(ps.getUntrackedParameter<std::vector<std::string> >("resolutionSummaryPrefixes")) {

  efficiency_calculator_ = std::make_unique<GEMDQMEfficiencyCalculator>();
}

GEMEfficiencyHarvester::~GEMEfficiencyHarvester() {}

void GEMEfficiencyHarvester::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.addUntracked<std::string>("folder", "GEM/Efficiency/Muon");
  desc.addUntracked<std::string>("logCategory", "GEMEfficiencyHarvester");
  desc.addUntracked<std::vector<std::string> >("resolutionSummaryPrefixes", {"rdphi"});
  descriptions.add("gemEfficiencyHarvester", desc);
}

void GEMEfficiencyHarvester::doEfficiency(DQMStore::IBooker& ibooker, DQMStore::IGetter& igetter) {
  const std::string efficiency_folder = kFolder_ + "/Efficiency/";
  efficiency_calculator_->drawEfficiency(ibooker, igetter, efficiency_folder);
}

std::vector<std::string> GEMEfficiencyHarvester::splitString(std::string name, const std::string delimiter) {
  std::vector<std::string> tokens;
  size_t delimiter_pos;
  size_t delimiter_len = delimiter.length();
  while ((delimiter_pos = name.find(delimiter)) != std::string::npos) {
    tokens.push_back(name.substr(0, delimiter_pos));
    name.erase(0, delimiter_pos + delimiter_len);
  }
  tokens.push_back(name);
  return tokens;
}

std::tuple<std::string, int, int> GEMEfficiencyHarvester::parseResidualName(const std::string org_name,
                                                                            const std::string prefix) {

  if (not TPRegexp("\\w+_GE\\d1-(P|M)-E\\d").MatchB(org_name)) {
    edm::LogError(kLogCategory_) << "";
    return std::make_tuple("", -1, -1);
  }

  std::string name = org_name;
  // e.g. "rdphi_GE21-P-E4" -> "_GE21-P-E4"
  name.erase(name.find(prefix), prefix.length());
  // "GE21-P-R4" -> "21-P-R4"
  name.erase(name.find("_GE"), 3);
  // "21-P-E4" -> {"11", "P", "E4"}
  const std::vector<std::string> tokens = splitString(name, "-");
  if (tokens.size() != 3) {
    return std::make_tuple("", -1, -1);
  }
  // "21" -> "2"
  const TString station_str = tokens.at(0).substr(0, 1);
  // "P" or "M"
  const std::string region = tokens.at(1);
  // E'4' or E'16'
  const TString ieta_str = tokens.at(2).substr(1);

  const int station = station_str.IsDigit() ? station_str.Atoi() : -1;
  const int ieta = ieta_str.IsDigit() ? ieta_str.Atoi() : -1;
  return std::make_tuple(region, station, ieta);
}

void GEMEfficiencyHarvester::doResolution(DQMStore::IBooker& ibooker,
                                          DQMStore::IGetter& igetter,
                                          const std::string prefix) {
  const std::string resolution_folder = kFolder_ + "/Resolution/";

  igetter.setCurrentFolder(resolution_folder);
  ibooker.setCurrentFolder(resolution_folder);

  // (histogram, (region_sign, station), ieta)
  std::vector<std::tuple<const TH1F*, std::pair<std::string, int>, int> > hist_vector;
  // (region_sign, station)
  std::vector<std::pair<std::string, int> > re_st_vec;
  // ieta
  std::vector<int> ieta_vec;

  for (const std::string& name : igetter.getMEs()) {
    if (name.find(prefix) == std::string::npos)
      continue;

    const std::string fullpath = resolution_folder + name;
    const MonitorElement* me = igetter.get(fullpath);
    if (me == nullptr) {
      edm::LogError(kLogCategory_) << "failed to get " << fullpath;
      continue;
    }

    const TH1F* hist = me->getTH1F();
    if (hist == nullptr) {
      edm::LogError(kLogCategory_) << "failed to get TH1F";
      continue;
    }

    const auto [region_sign, station, ieta] = parseResidualName(name, prefix);
    if (region_sign.empty() or station < 0 or ieta < 0) {
      edm::LogError(kLogCategory_) << "failed to parse the name of the residual histogram: " << name;
      continue;
    }
    std::pair<std::string, int> region_station(region_sign, station);

    hist_vector.emplace_back(hist, region_station, ieta);
    if (std::find(re_st_vec.begin(), re_st_vec.end(), region_station) == re_st_vec.end())
      re_st_vec.push_back(region_station);
    if (std::find(ieta_vec.begin(), ieta_vec.end(), ieta) == ieta_vec.end())
      ieta_vec.push_back(ieta);
  }  // MonitorElement

  if (hist_vector.empty()) {
    edm::LogError(kLogCategory_) << "failed to find " << prefix;
    return;
  }

  // NOTE
  // GE21-M, GE11-M, GE01-M, GE01-P, GE11-P, GE21-P
  auto f_sort = [](const std::pair<std::string, int>& lhs, const std::pair<std::string, int>& rhs) -> bool {
    if (lhs.first == rhs.first) {
      if (lhs.first == "M")
        return lhs.second > rhs.second;
      else
        return lhs.second < rhs.second;

    } else {
      return (lhs.first == "P");
    }
  };

  std::sort(re_st_vec.begin(), re_st_vec.end(), f_sort);
  std::sort(ieta_vec.begin(), ieta_vec.end());

  const int num_st = re_st_vec.size();
  const int num_ieta = ieta_vec.size();

  // NOTE
  TString tmp_title{std::get<0>(hist_vector.front())->GetTitle()};

  const TObjArray* tokens = tmp_title.Tokenize(":");
  const TString title_prefix = dynamic_cast<TObjString*>(tokens->At(0))->GetString();

  const TString h_mean_name = prefix + "_mean";
  const TString h_stddev_name = prefix + "_stddev";
  const TString h_skewness_name = prefix + "_skewness";

  const TString h_mean_title = title_prefix + " : Mean";
  const TString h_stddev_title = title_prefix + " : Standard Deviation";
  const TString h_skewness_title = title_prefix + " : Skewness";

  TH2F* h_mean = new TH2F(h_mean_name, h_mean_title, num_ieta, 0.5, num_ieta + 0.5, num_st, 0.5, num_st + 0.5);
  // x-axis
  h_mean->GetXaxis()->SetTitle("i#eta");
  for (unsigned int idx = 0; idx < ieta_vec.size(); idx++) {
    const int xbin = idx + 1;
    const char* label = Form("%d", ieta_vec[idx]);
    h_mean->GetXaxis()->SetBinLabel(xbin, label);
  }
  // y-axis
  for (unsigned int idx = 0; idx < re_st_vec.size(); idx++) {
    auto [region_sign, station] = re_st_vec[idx];
    const char* label = Form("GE%s%d/1", region_sign.c_str(), station);
    const int ybin = idx + 1;
    h_mean->GetYaxis()->SetBinLabel(ybin, label);
  }

  TH2F* h_stddev = dynamic_cast<TH2F*>(h_mean->Clone(h_stddev_name));
  TH2F* h_skewness = dynamic_cast<TH2F*>(h_mean->Clone(h_skewness_name));

  h_stddev->SetTitle(h_stddev_title);
  h_skewness->SetTitle(h_skewness_title);

  // NOTE
  for (auto [hist, region_station, ieta] : hist_vector) {
    const int xbin = findResolutionBin(ieta, ieta_vec);
    if (xbin < 0) {
      edm::LogError(kLogCategory_) << "found a wrong x bin = " << xbin;
      continue;
    }

    const int ybin = findResolutionBin(region_station, re_st_vec);
    if (ybin < 0) {
      edm::LogError(kLogCategory_) << "found a wrong y bin = " << ybin;
      continue;
    }

    h_mean->SetBinContent(xbin, ybin, hist->GetMean());
    h_stddev->SetBinContent(xbin, ybin, hist->GetStdDev());

    // FIXME
    // `GetSkewness` seems to returns nan when its histogram has no entry..
    const double skewness = hist->GetSkewness();
    if (edm::isFinite(skewness))
      h_skewness->SetBinContent(xbin, ybin, skewness);

    h_mean->SetBinError(xbin, ybin, hist->GetMeanError());
    h_stddev->SetBinError(xbin, ybin, hist->GetStdDevError());
    h_skewness->SetBinError(xbin, ybin, hist->GetSkewness(11));
  }

  for (auto& each : {h_mean, h_stddev, h_skewness}) {
    ibooker.book2D(each->GetName(), each);
  }
}

void GEMEfficiencyHarvester::dqmEndJob(DQMStore::IBooker& ibooker, DQMStore::IGetter& igetter) {
  doEfficiency(ibooker, igetter);
  for (const std::string& prefix : kResolutionSummaryPrefixes_) {
    doResolution(ibooker, igetter, prefix);
  }
}
