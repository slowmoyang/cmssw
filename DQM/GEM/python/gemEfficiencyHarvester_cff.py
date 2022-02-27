import FWCore.ParameterSet.Config as cms

from DQM.GEM.gemEfficiencyHarvester_cfi import gemEfficiencyHarvester
from DQM.GEM.gemEfficiencyAnalyzer_cff import gemEfficiencyAnalyzerTightGlb as _gemEfficiencyAnalyzerTightGlb
from DQM.GEM.gemEfficiencyAnalyzer_cff import gemEfficiencyAnalyzerSta as _gemEfficiencyAnalyzerSta
from DQM.GEM.gemEfficiencyAnalyzer_cff import gemEfficiencyAnalyzerTrk as _gemEfficiencyAnalyzerTrk

gemEfficiencyHarvesterTightGlb = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerTightGlb.folder.value()
)

gemEfficiencyHarvesterSta = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerSta.folder.value()
)

gemEfficiencyHarvesterTrk = gemEfficiencyHarvester.clone(
    folder = _gemEfficiencyAnalyzerTrk.folder.value()
)

gemEfficiencyClientsTask = cms.Task(
    gemEfficiencyHarvesterTightGlb,
    gemEfficiencyHarvesterSta,
)

gemEfficiencyClients = cms.Sequence(gemEfficiencyClientsTask)

gemEfficiencyAllClientsTask = cms.Task(
    gemEfficiencyHarvesterTightGlb,
    gemEfficiencyHarvesterSta,
    gemEfficiencyHarvesterTrk,
)

gemEfficiencyAllClients = cms.Sequence(gemEfficiencyAllClientsTask)
