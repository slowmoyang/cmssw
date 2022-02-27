import FWCore.ParameterSet.Config as cms
from DQM.GEM.gemEfficiencyAnalyzer_cfi import *

gemEfficiencyAnalyzerCosmics = gemEfficiencyAnalyzer.clone(
    isCosmics = True,
    muonPtMinCut = -1.0,
    rdphiCut = 5.0
)

gemEfficiencyAnalyzerCosmicsGlb = gemEfficiencyAnalyzerCosmics.clone(
    muonTrackType = 'CombinedTrack',
    folder = 'GEM/Efficiency/GLBMuon',
    name = 'Cosmic 2-Leg GLB Muon'
)

gemEfficiencyAnalyzerCosmicsSta = gemEfficiencyAnalyzerCosmics.clone(
    muonTrackType = 'OuterTrack',
    folder = 'GEM/Efficiency/STAMuon',
    name = 'Cosmic 2-Leg STA Muon'
)

gemEfficiencyAnalyzerCosmicsTrk = gemEfficiencyAnalyzerCosmics.clone(
    muonTrackType = 'InnerTrack',
    folder = 'GEM/Efficiency/TRKMuon',
    name = 'Cosmic 2-Leg TRK Muon'
)

gemEfficiencySourcesCosmicsTask = cms.Task(
    gemEfficiencyAnalyzerCosmicsGlb,
    gemEfficiencyAnalyzerCosmicsSta
)

gemEfficiencySourcesCosmics = cms.Sequence(gemEfficiencySourcesCosmicsTask)

gemEfficiencyAllSourcesCosmicsTask = cms.Task(
    gemEfficiencyAnalyzerCosmicsGlb,
    gemEfficiencyAnalyzerCosmicsSta,
    gemEfficiencyAnalyzerCosmicsTrk
)

gemEfficiencyAllSourcesCosmics = cms.Sequence(gemEfficiencyAllSourcesCosmicsTask)
