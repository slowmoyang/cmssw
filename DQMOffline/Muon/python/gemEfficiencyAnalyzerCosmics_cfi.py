import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from RecoMuon.TrackingTools.MuonServiceProxy_cff import MuonServiceProxy
from DQMOffline.Muon.gemEfficiencyAnalyzerCosmicsDefault_cfi import gemEfficiencyAnalyzerCosmicsDefault as _gemEfficiencyAnalyzerCosmicsDefault
from TrackingTools.TransientTrack.TransientTrackBuilder_cfi import *

gemEffCosmicsTightGlbMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag("muons"),
    cut = cms.string(
        "isGlobalMuon"
        "&& globalTrack.isNonnull"
        "&& passed(\"CutBasedIdTight\")"
    ),
)

gemEffCosmicsGlbMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag("muons"),
    cut = cms.string(
        "isGlobalMuon"
        "&& globalTrack.isNonnull"
    ),
)

gemEffCosmicsStaMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag("muons"),
    cut = cms.string(
        "isStandAloneMuon"
        "&& outerTrack.isNonnull"
    ),
)

gemEfficiencyAnalyzerCosmicsTightGlb = _gemEfficiencyAnalyzerCosmicsDefault.clone(
    ServiceParameters=MuonServiceProxy.ServiceParameters.clone(),
    muonTag="gemEffCosmicsTightGlbMuons",
    muonTrackType="CombinedTrack",
    muonName="Cosmic 2-Leg Tight Global Muon",
    folder="GEM/Efficiency/TightGlobalMuon",
)

gemEfficiencyAnalyzerCosmicsGlb = _gemEfficiencyAnalyzerCosmicsDefault.clone(
    ServiceParameters=MuonServiceProxy.ServiceParameters.clone(),
    muonTag="gemEffCosmicsGlbMuons",
    muonTrackType="CombinedTrack",
    muonName="Cosmic 2-Leg Global Muon",
    folder="GEM/Efficiency/GlobalMuon",
)

gemEfficiencyAnalyzerCosmicsSta = _gemEfficiencyAnalyzerCosmicsDefault.clone(
    ServiceParameters=MuonServiceProxy.ServiceParameters.clone(),
    muonTag="gemEffCosmicsStaMuons",
    muonTrackType="OuterTrack",
    muonName="Cosmic 2-Leg Standalone Muon",
    folder="GEM/Efficiency/StandaloneMuon",
)

from Configuration.Eras.Modifier_phase2_GEM_cff import phase2_GEM
phase2_GEM.toModify(gemEfficiencyAnalyzerCosmicsTightGlb,
    etaNbins=15,
    etaUp=3.0
)

phase2_GEM.toModify(gemEfficiencyAnalyzerCosmicsGlb,
    etaNbins=15,
    etaUp=3.0
)

phase2_GEM.toModify(gemEfficiencyAnalyzerCosmicsSta,
    etaNbins=15,
    etaUp=3.0
)

gemEfficiencyAnalyzerCosmicsTightGlbSeq = cms.Sequence(
    cms.ignore(gemEffCosmicsTightGlbMuons) *
    gemEfficiencyAnalyzerCosmicsTightGlb
)

gemEfficiencyAnalyzerCosmicsGlbSeq = cms.Sequence(
    cms.ignore(gemEffCosmicsGlbMuons) *
    gemEfficiencyAnalyzerCosmicsGlb
)

gemEfficiencyAnalyzerCosmicsStaSeq = cms.Sequence(
    cms.ignore(gemEffCosmicsStaMuons) *
    gemEfficiencyAnalyzerCosmicsSta
)
