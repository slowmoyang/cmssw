import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDHarvester import DQMEDHarvester
from DQMOffline.Muon.gemEfficiencyHarvesterDefault_cfi import gemEfficiencyHarvesterDefault as _gemEfficiencyHarvesterDefault
from DQMOffline.Muon.gemEfficiencyAnalyzer_cfi import gemEfficiencyAnalyzerTightGlb as _gemEfficiencyAnalyzerTightGlb
from DQMOffline.Muon.gemEfficiencyAnalyzer_cfi import gemEfficiencyAnalyzerSta as _gemEfficiencyAnalyzerSta
from DQMOffline.Muon.gemEfficiencyAnalyzer_cfi import gemEfficiencyAnalyzerTracker as _gemEfficiencyAnalyzerTracker

gemEfficiencyHarvesterTightGlb = _gemEfficiencyHarvesterDefault.clone(
    folder = _gemEfficiencyAnalyzerTightGlb.folder.value()
)

gemEfficiencyHarvesterSta = _gemEfficiencyHarvesterDefault.clone(
    folder = _gemEfficiencyAnalyzerSta.folder.value()
)

gemEfficiencyHarvesterTracker = _gemEfficiencyHarvesterDefault.clone(
    folder = _gemEfficiencyAnalyzerTracker.folder.value()
)
