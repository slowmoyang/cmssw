import FWCore.ParameterSet.Config as cms

from DQM.GEM.gemEfficiencyHarvester_cfi import gemEfficiencyHarvester
from DQM.GEM.gemEfficiencyAnalyzerCosmics_cff import gemEfficiencyAnalyzerCosmicsGlb as _gemEfficiencyAnalyzerCosmicsGlb
from DQM.GEM.gemEfficiencyAnalyzerCosmics_cff import gemEfficiencyAnalyzerCosmicsSta as _gemEfficiencyAnalyzerCosmicsSta

gemEfficiencyHarvesterCosmicsGlb = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerCosmicsGlb.folder.value()
)

gemEfficiencyHarvesterCosmicsSta = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerCosmicsSta.folder.value()
)
