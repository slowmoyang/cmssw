import FWCore.ParameterSet.Config as cms
from RecoMuon.TrackingTools.MuonServiceProxy_cff import MuonServiceProxy
from DQMOffline.Muon.GE11EffME11SegmentAnalyzerDefault_cfi import GE11EffME11SegmentAnalyzerDefault as _GE11EffME11SegmentAnalyzerDefault
from DQMOffline.Muon.GE11EffME11SegmentAnalyzerV2Default_cfi import GE11EffME11SegmentAnalyzerV2Default as _GE11EffME11SegmentAnalyzerV2Default
from TrackingTools.TransientTrack.TransientTrackBuilder_cfi import *

ge11EffMuons = cms.EDFilter("MuonSelector",
    src = cms.InputTag('muons'),
    cut = cms.string(
        '(pt > 3)'
    ),
    filter = cms.bool(False)
)


ge11EffME11SegmentAnalyzer_useFiducialCut_useME11A = _GE11EffME11SegmentAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = 'ge11EffMuons',
    useFiducialCut = True,
    useME11A = True,
    folder="GEM/Efficiency/ME11SegmentV1_useFiducialCut_useME11A"
)

ge11EffME11SegmentAnalyzer_useFiducialCut = _GE11EffME11SegmentAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = 'ge11EffMuons',
    useFiducialCut = True,
    useME11A = False,
    folder="GEM/Efficiency/ME11SegmentV1_useFiducialCut"
)

ge11EffME11SegmentAnalyzer = _GE11EffME11SegmentAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = 'ge11EffMuons',
    useFiducialCut = False,
    useME11A = False,
    folder="GEM/Efficiency/ME11SegmentV1"
)

ge11EffME11SegmentAnalyzerV2_checkBoundsWithPropError = _GE11EffME11SegmentAnalyzerV2Default.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = 'ge11EffMuons',
    checkBoundsWithPropError=True,
    checkBoundsWithPropErrorScale = -1.0,
    folder="GEM/Efficiency/ME11SegmentV2_checkBoundsWithPropError",
)

ge11EffME11SegmentAnalyzerV2_checkBoundsWithPropError_scale2 = _GE11EffME11SegmentAnalyzerV2Default.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = 'ge11EffMuons',
    checkBoundsWithPropError = True,
    checkBoundsWithPropErrorScale = -2.0,
    folder="GEM/Efficiency/ME11SegmentV2_checkBoundsWithPropError_scale2",
)

ge11EffME11SegmentAnalyzerV2 = _GE11EffME11SegmentAnalyzerV2Default.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = 'ge11EffMuons',
    checkBoundsWithPropError = False,
    folder="GEM/Efficiency/ME11SegmentV2",
)



ge11EffME11SegmentAnalyzerSeq = cms.Sequence(
    cms.ignore(ge11EffMuons) *
    ge11EffME11SegmentAnalyzer_useFiducialCut_useME11A *
    ge11EffME11SegmentAnalyzer_useFiducialCut *
    ge11EffME11SegmentAnalyzer *
    ge11EffME11SegmentAnalyzerV2_checkBoundsWithPropError *
    ge11EffME11SegmentAnalyzerV2_checkBoundsWithPropError_scale2 *
    ge11EffME11SegmentAnalyzerV2
)
