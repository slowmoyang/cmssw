#ifndef DQMOffline_Muon_GEMOfflineMonitor_h
#define DQMOffline_Muon_GEMOfflineMonitor_h

#include "DQMOffline/Muon/interface/GEMOfflineDQMBase.h"

#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

class GEMOfflineMonitor : public GEMOfflineDQMBase {
public:
  explicit GEMOfflineMonitor(const edm::ParameterSet &);
  ~GEMOfflineMonitor() override;
  static void fillDescriptions(edm::ConfigurationDescriptions &);

protected:
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
  void analyze(const edm::Event &event, const edm::EventSetup &eventSetup) override;

private:
  void bookDigiOccupancy(DQMStore::IBooker &, const edm::ESHandle<GEMGeometry> &);
  void bookHitOccupancy(DQMStore::IBooker &, const edm::ESHandle<GEMGeometry> &);

  void doDigiOccupancy(const edm::ESHandle<GEMGeometry> &, const edm::Handle<GEMDigiCollection> &);
  void doHitOccupancy(const edm::ESHandle<GEMGeometry> &, const edm::Handle<GEMRecHitCollection> &);

  // const member initialized in the Constructor initializer list
  const edm::ESGetToken<GEMGeometry, MuonGeometryRecord> kGEMTokenBeginRun_;
  const edm::ESGetToken<GEMGeometry, MuonGeometryRecord> kGEMToken_;
  const edm::EDGetTokenT<GEMDigiCollection> kGEMDigiToken_;
  const edm::EDGetTokenT<GEMRecHitCollection> kGEMRecHitToken_;
  const bool kDoDigiOccupancy_;
  const bool kDoHitOccupancy_;

  // MonitorElement
  MEMap me_digi_det_;  // TH2F, region-station
  MEMap me_hit_det_;   // TH2F, region-station

  // const
  const std::string kLogCategory_ = "GEMOfflineMonitor";
};

#endif  // DQMOffline_Muon_GEMOfflineMonitor_h
