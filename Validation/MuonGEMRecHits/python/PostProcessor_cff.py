import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDHarvester import DQMEDHarvester

gemRecHitHarvesting = DQMEDHarvester("MuonGEMRecHitsHarvestor",
    regionIds=cms.untracked.vint32(-1, 1), 
    stationIds=cms.untracked.vint32(1, 2), 
    layerIds=cms.untracked.vint32(1, 2),
    logCategory=cms.untracked.string("MuonGEMRecHitsHarvestor"),
    folder=cms.string("MuonGEMRecHitsV/GEMRecHitsTask/"),
)
MuonGEMRecHitsPostProcessors = cms.Sequence(gemRecHitHarvesting) 
