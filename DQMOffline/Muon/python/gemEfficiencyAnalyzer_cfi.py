import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from RecoMuon.TrackingTools.MuonServiceProxy_cff import MuonServiceProxy
from DQMOffline.Muon.gemEfficiencyAnalyzerDefault_cfi import gemEfficiencyAnalyzerDefault as _gemEfficiencyAnalyzerDefault
from TrackingTools.TransientTrack.TransientTrackBuilder_cfi import *

gemEffTightGlbMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag("muons"),
    cut = cms.string(
        "(pt > 20)"
        "&& isGlobalMuon"
        "&& globalTrack.isNonnull"
        "&& passed(\"CutBasedIdTight\")"
    ),
)

gemEffStaMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag("muons"),
    cut = cms.string(
        "(pt > 20)"
        "&& isStandAloneMuon"
        "&& outerTrack.isNonnull"
    ),
)

gemEfficiencyAnalyzerTightGlb = _gemEfficiencyAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = cms.InputTag("gemEffTightGlbMuons"),
    muonTrackType = cms.untracked.string("CombinedTrack"),
    muonName = cms.untracked.string("Tight GLB Muon"),
    folder = cms.untracked.string("GEM/Efficiency/type1"),
)

gemEfficiencyAnalyzerSta = _gemEfficiencyAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = cms.InputTag("gemEffStaMuons"),
    muonTrackType = cms.untracked.string("OuterTrack"),
    muonName = cms.untracked.string("STA Muon"),
    folder = cms.untracked.string("GEM/Efficiency/type2"),
)

from Configuration.Eras.Modifier_phase2_GEM_cff import phase2_GEM
phase2_GEM.toModify(
    gemEfficiencyAnalyzerTightGlb,
    etaNbins=cms.untracked.int32(15),
    etaUp=cms.untracked.double(3.0))

phase2_GEM.toModify(
    gemEfficiencyAnalyzerSta,
    etaNbins=cms.untracked.int32(15),
    etaUp=cms.untracked.double(3.0))

gemEfficiencyAnalyzerTightGlbSeq = cms.Sequence(
    cms.ignore(gemEffTightGlbMuons) *
    gemEfficiencyAnalyzerTightGlb)

gemEfficiencyAnalyzerStaSeq = cms.Sequence(
    cms.ignore(gemEffStaMuons) *
    gemEfficiencyAnalyzerSta)
