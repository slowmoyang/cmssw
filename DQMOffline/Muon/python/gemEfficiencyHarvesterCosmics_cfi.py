import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDHarvester import DQMEDHarvester
from DQMOffline.Muon.gemEfficiencyHarvesterDefault_cfi import gemEfficiencyHarvesterDefault as _gemEfficiencyHarvesterDefault
from DQMOffline.Muon.gemEfficiencyAnalyzerCosmics_cfi import gemEfficiencyAnalyzerCosmics as _gemEfficiencyAnalyzerCosmics
from DQMOffline.Muon.gemEfficiencyAnalyzerCosmics_cfi import gemEfficiencyAnalyzerCosmicsSta as _gemEfficiencyAnalyzerCosmicsSta
from DQMOffline.Muon.gemEfficiencyAnalyzerCosmics_cfi import gemEfficiencyAnalyzerCosmicsTracker as _gemEfficiencyAnalyzerCosmicsTracker

gemEfficiencyHarvesterCosmicsGlb = _gemEfficiencyHarvesterDefault.clone(
    folder = _gemEfficiencyAnalyzerCosmicsGlb.folder.value(),
)

gemEfficiencyHarvesterCosmicsSta = _gemEfficiencyHarvesterDefault.clone(
    folder = _gemEfficiencyAnalyzerCosmicsSta.folder.value(),
)

gemEfficiencyHarvesterCosmicsTracker = _gemEfficiencyHarvesterDefault.clone(
    folder = _gemEfficiencyAnalyzerCosmicsTracker.folder.value(),
)
