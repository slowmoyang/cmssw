import FWCore.ParameterSet.Config as cms
from RecoMuon.TrackingTools.MuonServiceProxy_cff import MuonServiceProxy
from DQMOffline.Muon.GE11EffME11SegmentAnalyzerDefault_cfi import GE11EffME11SegmentAnalyzerDefault as _GE11EffME11SegmentAnalyzerDefault
from TrackingTools.TransientTrack.TransientTrackBuilder_cfi import *

ge11EffME11SegmentAnalyzer = _GE11EffME11SegmentAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
)

ge11EffME11SegmentAnalyzerSeq = cms.Sequence(
    ge11EffME11SegmentAnalyzer
)
