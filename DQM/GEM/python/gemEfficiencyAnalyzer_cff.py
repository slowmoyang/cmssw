import FWCore.ParameterSet.Config as cms
from DQM.GEM.gemEfficiencyAnalyzer_cfi import *

gemDQMTightGlbMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag('muons'),
    cut = cms.string(
        'isGlobalMuon'
        '&& globalTrack.isNonnull'
        '&& passed(\'CutBasedIdTight\')'
    ),
    filter = cms.bool(False)
)

gemEfficiencyAnalyzerTightGlb = gemEfficiencyAnalyzer.clone(
    muonTag = 'gemDQMTightGlbMuons',
    muonTrackType = 'CombinedTrack',
    folder = 'GEM/Efficiency/GLBMuon',
    name = 'Tight GLB Muon',
)

gemEfficiencyAnalyzerSta = gemEfficiencyAnalyzer.clone(
    muonTrackType = 'OuterTrack',
    folder = 'GEM/Efficiency/STAMuon',
    name = 'STA Muon',
)

gemEfficiencyAnalyzerTightGlbSeq = cms.Sequence(
    cms.ignore(gemDQMTightGlbMuons) *
    gemEfficiencyAnalyzerTightGlb)
