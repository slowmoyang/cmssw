import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDHarvester import DQMEDHarvester

gemDigiStripHarvesting = DQMEDHarvester("MuonGEMDigisHarvestor",
    dbePath = cms.untracked.string("MuonGEMDigisV/GEMDigisTask/"),
    stripDigiPath = cms.string("MuonGEMDigisV/GEMDigisTask/Strip/"),
    detailPlot = cms.bool(True), 
    regionIds = cms.untracked.vint32(-1, 1),                                           
    stationIds = cms.untracked.vint32(1, 2), 
    layerIds = cms.untracked.vint32(1, 2),
)

MuonGEMDigisPostProcessors = cms.Sequence( gemDigiStripHarvesting ) 
