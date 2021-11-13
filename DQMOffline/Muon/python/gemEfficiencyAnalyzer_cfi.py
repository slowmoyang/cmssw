import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from RecoMuon.TrackingTools.MuonServiceProxy_cff import MuonServiceProxy
from DQMOffline.Muon.gemEfficiencyAnalyzerDefault_cfi import gemEfficiencyAnalyzerDefault as _gemEfficiencyAnalyzerDefault
from TrackingTools.TransientTrack.TransientTrackBuilder_cfi import *

gemOfflineDQMTightGlbMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag('muons'),
    cut = cms.string(
        '(pt > 20)'
        '&& isGlobalMuon'
        '&& globalTrack.isNonnull'
        '&& passed(\'CutBasedIdTight\')'
    ),
    filter = cms.bool(False)
)

gemOfflineDQMStaMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag('muons'),
    cut = cms.string(
        '(pt > 20)'
        '&& isStandAloneMuon'
        '&& outerTrack.isNonnull'
    ),
    filter = cms.bool(False)
)

gemOfflineDQMTrackerMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag('muons'),
    cut = cms.string(
        '(pt > 20)'
        '&& isTrackerMuon'
        '&& innerTrack.isNonnull'
    ),
    filter = cms.bool(False)
)

gemEfficiencyAnalyzerTightGlb = _gemEfficiencyAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = 'gemOfflineDQMTightGlbMuons',
    muonTrackType = 'CombinedTrack',
    name = 'Tight GLB Muon',
    folder = 'GEM/Efficiency/GLB',
)

gemEfficiencyAnalyzerSta = _gemEfficiencyAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = "gemOfflineDQMStaMuons",
    muonTrackType = 'OuterTrack',
    name = 'STA Muon',
    folder = 'GEM/Efficiency/STA',
)

gemEfficiencyAnalyzerTracker = _gemEfficiencyAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = "gemOfflineDQMTrackerMuons",
    muonTrackType = 'InnerTrack',
    name = 'Tracker Muon',
    folder = 'GEM/Efficiency/TRK',
)

from Configuration.Eras.Modifier_phase2_GEM_cff import phase2_GEM
phase2_GEM.toModify(gemEfficiencyAnalyzerTightGlb, etaNbins=15, etaUp=3.0)
phase2_GEM.toModify(gemEfficiencyAnalyzerSta, etaNbins=15, etaUp=3.0)
phase2_GEM.toModify(gemEfficiencyAnalyzerTracker, etaNbins=15, etaUp=3.0)

gemEfficiencyAnalyzerTightGlbSeq = cms.Sequence(
    cms.ignore(gemOfflineDQMTightGlbMuons) *
    gemEfficiencyAnalyzerTightGlb)

gemEfficiencyAnalyzerStaSeq = cms.Sequence(
    cms.ignore(gemOfflineDQMStaMuons) *
    gemEfficiencyAnalyzerSta)

gemEfficiencyAnalyzerTrackerSeq = cms.Sequence(
    cms.ignore(gemOfflineDQMTrackerMuons) *
    gemEfficiencyAnalyzerTracker)
