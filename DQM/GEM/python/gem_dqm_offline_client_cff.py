import FWCore.ParameterSet.Config as cms

from DQM.GEM.GEMDQMHarvester_cfi import *
from DQM.GEM.gemEfficiencyHarvester_cff import *
from DQM.GEM.gemEffByGEMCSCSegmentClient_cff import *

gemClients = cms.Sequence(
    GEMDQMHarvester *
    gemEfficiencyHarvesterTightGlb *
    gemEfficiencyHarvesterSta
)
