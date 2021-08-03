import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDHarvester import DQMEDHarvester
from DQMOffline.Muon.gemEfficiencyHarvesterDefault_cfi import gemEfficiencyHarvesterDefault as _gemEfficiencyHarvesterDefault
from DQMOffline.Muon.gemEfficiencyAnalyzerCosmics_cfi import gemEfficiencyAnalyzerCosmicsTightGlb as _gemEfficiencyAnalyzerCosmicsTightGlb
from DQMOffline.Muon.gemEfficiencyAnalyzerCosmics_cfi import gemEfficiencyAnalyzerCosmicsGlb as _gemEfficiencyAnalyzerCosmicsGlb
from DQMOffline.Muon.gemEfficiencyAnalyzerCosmics_cfi import gemEfficiencyAnalyzerCosmicsSta as _gemEfficiencyAnalyzerCosmicsSta

gemEfficiencyHarvesterCosmicsTightGlb = _gemEfficiencyHarvesterDefault.clone(
    folder=_gemEfficiencyAnalyzerCosmicsTightGlb.folder.value(),
)

gemEfficiencyHarvesterCosmicsGlb = _gemEfficiencyHarvesterDefault.clone(
    folder=_gemEfficiencyAnalyzerCosmicsGlb.folder.value(),
)

gemEfficiencyHarvesterCosmicsSta = _gemEfficiencyHarvesterDefault.clone(
    folder=_gemEfficiencyAnalyzerCosmicsSta.folder.value(),
)
