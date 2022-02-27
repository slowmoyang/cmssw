import FWCore.ParameterSet.Config as cms

from DQM.GEM.gemEfficiencyHarvester_cfi import gemEfficiencyHarvester
from DQM.GEM.gemEfficiencyAnalyzerCosmics_cff import gemEfficiencyAnalyzerCosmicsGlb as _gemEfficiencyAnalyzerCosmicsGlb
from DQM.GEM.gemEfficiencyAnalyzerCosmics_cff import gemEfficiencyAnalyzerCosmicsSta as _gemEfficiencyAnalyzerCosmicsSta
from DQM.GEM.gemEfficiencyAnalyzerCosmics_cff import gemEfficiencyAnalyzerCosmicsTrk as _gemEfficiencyAnalyzerCosmicsTrk

gemEfficiencyHarvesterCosmicsGlb = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerCosmicsGlb.folder.value()
)

gemEfficiencyHarvesterCosmicsSta = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerCosmicsSta.folder.value()
)

gemEfficiencyHarvesterCosmicsTrk = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerCosmicsTrk.folder.value()
)

gemEfficiencyClientsCosmicsTask = cms.Task(
    gemEfficiencyHarvesterCosmicsGlb,
    gemEfficiencyHarvesterCosmicsSta,
)

gemEfficiencyClientsCosmics = cms.Sequence(gemEfficiencyClientsCosmicsTask)

gemEfficiencyAllClientsCosmicsTask = cms.Task(
    gemEfficiencyHarvesterCosmicsGlb,
    gemEfficiencyHarvesterCosmicsSta,
    gemEfficiencyHarvesterCosmicsTrk,
)

gemEfficiencyAllClientsCosmics = cms.Sequence(gemEfficiencyAllClientsCosmicsTask)
