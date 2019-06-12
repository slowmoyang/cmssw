import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDHarvester import DQMEDHarvester

gemSimHarvesting = DQMEDHarvester("MuonGEMHitsHarvestor",
    logCategory=cms.untracked.string("MuonGEMHitsHarvestor"),
    folder=cms.string("MuonGEMHitsV/GEMHitsTask"),
)
MuonGEMHitsPostProcessors = cms.Sequence( gemSimHarvesting ) 
