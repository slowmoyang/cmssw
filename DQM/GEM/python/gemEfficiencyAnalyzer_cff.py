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

gemOfflineDQMQianyingTrkMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag('muons'),
    cut = cms.string(
        'isTrackerMuon'
        '&& (innerTrack.normalizedChi2 < 10)'
        '&& (innerTrack.hitPattern.numberOfValidPixelHits > 0)'
        '&& (innerTrack.hitPattern.trackerLayersWithMeasurement > 5)'
        '&& ((isolationR03.sumPt)/(pt) < 0.1)'
        '&& (pt>10.)'
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

gemEfficiencyAnalyzerQianyingTrk = gemEfficiencyAnalyzer.clone(
    muonTag = "gemOfflineDQMQianyingTrkMuons",
    muonTrackType = 'InnerTrack',
    folder = 'GEM/Efficiency/QianyingTRKMuon',
    name = 'TRK Muon with Qianying\'s cuts',
)


gemEfficiencyAnalyzerTightGlbSeq = cms.Sequence(
    cms.ignore(gemOfflineDQMTightGlbMuons) *
    gemEfficiencyAnalyzerTightGlb
)

gemEfficiencyAnalyzerQianyingTrkSeq = cms.Sequence(
    cms.ignore(gemOfflineDQMQianyingTrkMuons) *
    gemEfficiencyAnalyzerQianyingTrk
)

gemEfficiencySource = cms.Sequence(
    gemEfficiencyAnalyzerTightGlbSeq *
    gemEfficiencyAnalyzerSta *
    gemEfficiencyAnalyzerTrk *
    gemEfficiencyAnalyzerQianyingTrkSeq
)
