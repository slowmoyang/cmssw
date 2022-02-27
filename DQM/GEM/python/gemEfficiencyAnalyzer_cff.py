import FWCore.ParameterSet.Config as cms
from DQM.GEM.gemEfficiencyAnalyzer_cfi import *

gemOfflineDQMTightGlbMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag('muons'),
    cut = cms.string(
        'isGlobalMuon'
        '&& globalTrack.isNonnull'
        '&& passed(\'CutBasedIdTight\')'
    ),
    filter = cms.bool(False)
)

gemEfficiencyAnalyzerTightGlb = gemEfficiencyAnalyzer.clone(
    muonTag = 'gemOfflineDQMTightGlbMuons',
    muonTrackType = 'CombinedTrack',
    folder = 'GEM/Efficiency/GLBMuon',
    name = 'Tight GLB Muon',
)

gemEfficiencyAnalyzerSta = gemEfficiencyAnalyzer.clone(
    muonTag = "muons",
    muonTrackType = 'OuterTrack',
    folder = 'GEM/Efficiency/STAMuon',
    name = 'STA Muon',
)

gemEfficiencyAnalyzerTrk = gemEfficiencyAnalyzer.clone(
    muonTag = "muons",
    muonTrackType = 'InnerTrack',
    folder = 'GEM/Efficiency/TRKMuon',
    name = 'TRK Muon',
)

gemEfficiencyAnalyzerTightGlbSeq = cms.Sequence(
    cms.ignore(gemOfflineDQMTightGlbMuons) *
    gemEfficiencyAnalyzerTightGlb
)

gemEfficiencySources = cms.Sequence(
    gemEfficiencyAnalyzerTightGlbSeq *
    gemEfficiencyAnalyzerSta
)

gemEfficiencyAllSources = cms.Sequence(
    gemEfficiencyAnalyzerTightGlbSeq *
    gemEfficiencyAnalyzerSta *
    gemEfficiencyAnalyzerTrk
)
