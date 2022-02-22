import FWCore.ParameterSet.Config as cms

from DQM.GEM.gemEfficiencyHarvester_cfi import gemEfficiencyHarvester
from DQM.GEM.gemEfficiencyAnalyzer_cff import gemEfficiencyAnalyzerTightGlb as _gemEfficiencyAnalyzerTightGlb
from DQM.GEM.gemEfficiencyAnalyzer_cff import gemEfficiencyAnalyzerSta as _gemEfficiencyAnalyzerSta
from DQM.GEM.gemEfficiencyAnalyzer_cff import gemEfficiencyAnalyzerTrk as _gemEfficiencyAnalyzerTrk
from DQM.GEM.gemEfficiencyAnalyzer_cff import gemEfficiencyAnalyzerQianyingTrk as _gemEfficiencyAnalyzerQianyingTrk

gemEfficiencyHarvesterTightGlb = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerTightGlb.folder.value()
)

gemEfficiencyHarvesterSta = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerSta.folder.value()
)

gemEfficiencyHarvesterTrk = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerTrk.folder.value()
)

gemEfficiencyHarvesterQianyingTrk = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerQianyingTrk.folder.value()
)

gemEfficiencyClientTask = cms.Task(
    gemEfficiencyHarvesterTightGlb,
    gemEfficiencyHarvesterSta,
    gemEfficiencyHarvesterTrk,
    gemEfficiencyHarvesterQianyingTrk,
)

gemEfficiencyClient = cms.Sequence(gemEfficiencyClientTask)
