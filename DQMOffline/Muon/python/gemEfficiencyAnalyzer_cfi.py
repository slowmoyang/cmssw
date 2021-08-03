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

gemEffGlbMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag("muons"),
    cut = cms.string(
        "(pt > 20)"
        "&& isGlobalMuon"
        "&& globalTrack.isNonnull"
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
    muonTag="gemEffTightGlbMuons",
    muonTrackType="CombinedTrack",
    muonName="Tight Global Muon",
    folder="GEM/Efficiency/TightGlobalMuon",
)

gemEfficiencyAnalyzerGlb = _gemEfficiencyAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag="gemEffGlbMuons",
    muonTrackType="CombinedTrack",
    muonName="Global Muon",
    folder="GEM/Efficiency/GlobalMuon",
)

gemEfficiencyAnalyzerSta = _gemEfficiencyAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag="gemEffStaMuons",
    muonTrackType="OuterTrack",
    muonName="Standalone Muon",
    folder="GEM/Efficiency/StandaloneMuon",
)

from Configuration.Eras.Modifier_phase2_GEM_cff import phase2_GEM
phase2_GEM.toModify(gemEfficiencyAnalyzerTightGlb,
    etaNbins=15,
    etaUp=3.0
)

phase2_GEM.toModify(gemEfficiencyAnalyzerGlb,
    etaNbins=15,
    etaUp=3.0
)

phase2_GEM.toModify(gemEfficiencyAnalyzerSta,
    etaNbins=15,
    etaUp=3.0
)

gemEfficiencyAnalyzerTightGlbSeq = cms.Sequence(
    cms.ignore(gemEffTightGlbMuons) *
    gemEfficiencyAnalyzerTightGlb
)

gemEfficiencyAnalyzerGlbSeq = cms.Sequence(
    cms.ignore(gemEffGlbMuons) *
    gemEfficiencyAnalyzerGlb
)

gemEfficiencyAnalyzerStaSeq = cms.Sequence(
    cms.ignore(gemEffStaMuons) *
    gemEfficiencyAnalyzerSta
)
