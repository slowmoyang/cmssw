#include "DQMOffline/Muon/interface/GEMOfflineDQMBase.h"
#include "FWCore/Utilities/interface/Likely.h"

#include <algorithm>

GEMOfflineDQMBase::GEMOfflineDQMBase(const edm::ParameterSet& pset) {}

int GEMOfflineDQMBase::getDetOccXBin(const GEMDetId& gem_id, const edm::ESHandle<GEMGeometry>& gem) {
  const GEMSuperChamber* superchamber = gem->superChamber(gem_id);
  if (superchamber == nullptr) {
    return -1;
  }
  return getDetOccXBin(gem_id.chamber(), gem_id.layer(), superchamber->nChambers());
}

void GEMOfflineDQMBase::setDetLabelsVFAT(MonitorElement* me, const GEMStation* station) {
  if (me == nullptr) {
    edm::LogError(kLogCategory_) << "MonitorElement* is nullptr" << std::endl;
    return;
  }

  me->setAxisTitle("Superchamber / Chamber", 1);
  for (const GEMSuperChamber* superchamber : station->superChambers()) {
    const int num_chambers = superchamber->nChambers();
    for (const GEMChamber* chamber : superchamber->chambers()) {
      const int sc = chamber->id().chamber();
      const int ch = chamber->id().layer();
      const int xbin = getDetOccXBin(sc, ch, num_chambers);
      const char* label = Form("%d/%d", sc, ch);
      me->setBinLabel(xbin, label, 1);
    }
  }

  me->setAxisTitle("VFAT (i#eta)", 2);
  const int max_vfat = getMaxVFAT(station->station());
  if (max_vfat < 0) {
    edm::LogError(kLogCategory_) << "Wrong max VFAT: " << max_vfat << " at Station " << station->station() << std::endl;
    return;
  }

  const int num_etas = getNumEtaPartitions(station);
  for (int ieta = 1; ieta <= num_etas; ieta++) {
    for (int vfat_phi = 1; vfat_phi <= max_vfat; vfat_phi++) {
      const int ybin = getVFATNumber(station->station(), ieta, vfat_phi);
      const char* label = Form("%d (%d)", ybin, ieta);
      me->setBinLabel(ybin, label, 2);
    }
  }
}

void GEMOfflineDQMBase::setDetLabelsEta(MonitorElement* me, const GEMStation* station) {
  if (me == nullptr) {
    edm::LogError(kLogCategory_) << "MonitorElement* is nullptr" << std::endl;
    return;
  }

  me->setAxisTitle("Chamber", 1);
  for (const GEMSuperChamber* superchamber : station->superChambers()) {
    const int chamber = superchamber->id().chamber();
    const std::string&& label = std::to_string(chamber);
    me->setBinLabel(chamber, label, 1);
  }

  const int num_etas = getNumEtaPartitions(station);
  me->setAxisTitle("i#eta", 2);
  for (int ieta = 1; ieta <= num_etas; ieta++) {
    const std::string&& label = std::to_string(ieta);
    me->setBinLabel(ieta, label, 2);
  }
}

int GEMOfflineDQMBase::getNumEtaPartitions(const GEMStation* station) {
  const auto&& superchambers = station->superChambers();
  if (not checkRefs(superchambers)) {
    edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMSuperChamber ptrs" << std::endl;
    return 0;
  }

  const auto& chambers = superchambers.front()->chambers();
  if (not checkRefs(chambers)) {
    edm::LogError(kLogCategory_) << "failed to get a valid vector of GEMChamber ptrs" << std::endl;
    return 0;
  }

  return chambers.front()->nEtaPartitions();
}

void GEMOfflineDQMBase::fillMEWithinLimits(dqm::impl::MonitorElement* me, const double x) {
  const double xlow = me->getAxisMin(1);
  const double xup = me->getAxisMax(1) - kEps_;
  me->Fill(std::clamp(x, xlow, xup));
}

void GEMOfflineDQMBase::fillME(MEMap& me_map, const GEMDetId& key, const double x) {
  if UNLIKELY (me_map.find(key) == me_map.end()) {
    const std::string hint = me_map.empty() ? "empty" : me_map.begin()->second->getName();
    edm::LogError(kLogCategory_) << "got an invalid key: " << key << ", hint=" << hint << std::endl;

  } else {
    me_map[key]->Fill(x);
  }
}

void GEMOfflineDQMBase::fillMEWithinLimits(MEMap& me_map, const GEMDetId& key, const double x) {
  // https://github.com/cms-sw/cmssw/blob/CMSSW_12_0_0_pre3/DQMOffline/L1Trigger/src/L1TFillWithinLimits.cc
  if UNLIKELY (me_map.find(key) == me_map.end()) {
    const std::string hint = me_map.empty() ? "empty" : me_map.begin()->second->getName();
    edm::LogError(kLogCategory_) << "got an invalid key: " << key << ", hint=" << hint << std::endl;

  } else {
    fillMEWithinLimits(me_map[key], x);
  }
}

void GEMOfflineDQMBase::fillME(MEMap& me_map, const GEMDetId& key, const double x, const double y) {
  if UNLIKELY (me_map.find(key) == me_map.end()) {
    const std::string hint = me_map.empty() ? "empty" : me_map.begin()->second->getName();
    edm::LogError(kLogCategory_) << "got an invalid key: " << key << ", hint=" << hint << std::endl;

  } else {
    me_map[key]->Fill(x, y);
  }
}
