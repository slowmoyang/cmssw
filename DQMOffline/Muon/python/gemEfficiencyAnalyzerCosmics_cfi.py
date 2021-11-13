import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from RecoMuon.TrackingTools.MuonServiceProxy_cff import MuonServiceProxy
from DQMOffline.Muon.gemEfficiencyAnalyzerCosmicsDefault_cfi import gemEfficiencyAnalyzerCosmicsDefault as _gemEfficiencyAnalyzerCosmicsDefault
from TrackingTools.TransientTrack.TransientTrackBuilder_cfi import *

gemEfficiencyAnalyzerCosmicsGlb = _gemEfficiencyAnalyzerCosmicsDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = 'muons',
    muonTrackType = 'CombinedTrack',
    name = 'Cosmic 2-Leg GLB Muon',
    folder = 'GEM/Efficiency/GLB',
)

gemEfficiencyAnalyzerCosmicsSta = _gemEfficiencyAnalyzerCosmicsDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = 'muons',
    muonTrackType = 'OuterTrack',
    name = 'Cosmic 2-Leg STA Muon',
    folder = 'GEM/Efficiency/STA',
)

gemEfficiencyAnalyzerCosmicsTracker = _gemEfficiencyAnalyzerCosmicsDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
    muonTag = 'muons',
    muonTrackType = 'InnerTrack',
    name = 'Cosmic 2-Leg Tracker Muon',
    folder = 'GEM/Efficiency/TRK',
)

from Configuration.Eras.Modifier_phase2_GEM_cff import phase2_GEM
phase2_GEM.toModify(gemEfficiencyAnalyzerCosmicsGlb, etaNbins=15, etaUp=3.0)
phase2_GEM.toModify(gemEfficiencyAnalyzerCosmicsSta, etaNbins=15, etaUp=3.0)
phase2_GEM.toModify(gemEfficiencyAnalyzerCosmicsTracker, etaNbins=15, etaUp=3.0)
