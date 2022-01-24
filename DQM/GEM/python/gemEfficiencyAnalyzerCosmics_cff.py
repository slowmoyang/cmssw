import FWCore.ParameterSet.Config as cms
from DQM.GEM.gemEfficiencyAnalyzer_cfi import *

gemEfficiencyAnalyzerCosmics = gemEfficiencyAnalyzer.clone(
    isCosmics = True,
    muonPtMin = -1.0,
    residualRPhiCut = 5.0,
)

gemEfficiencyAnalyzerCosmicsGlb = gemEfficiencyAnalyzerCosmics.clone(
    muonTrackType = 'CombinedTrack',
    folder = 'GEM/Efficiency/GLBMuon',
    name = 'Cosmic 2-Leg GLB Muon',
)

gemEfficiencyAnalyzerCosmicsSta = gemEfficiencyAnalyzerCosmics.clone(
    muonTrackType = 'OuterTrack',
    folder = 'GEM/Efficiency/STAMuon',
    name = 'Cosmic 2-Leg STA Muon',
)
