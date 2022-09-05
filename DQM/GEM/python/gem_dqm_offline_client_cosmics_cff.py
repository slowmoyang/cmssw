import FWCore.ParameterSet.Config as cms

from DQM.GEM.GEMDQMHarvester_cfi import *
from DQM.GEM.gemEfficiencyHarvesterCosmics_cff import *
from DQM.GEM.gemEffByGEMCSCSegmentClient_cff import *

gemClientsCosmics = cms.Sequence(
    GEMDQMHarvester *
    gemEfficiencyHarvesterCosmicsGlb *
    gemEfficiencyHarvesterCosmicsSta
)
