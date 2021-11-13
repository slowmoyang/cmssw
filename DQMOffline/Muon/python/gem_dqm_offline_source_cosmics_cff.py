import FWCore.ParameterSet.Config as cms

from DQMOffline.Muon.gemEfficiencyAnalyzerCosmics_cfi import *

gemSourcesCosmics = cms.Sequence(
    gemEfficiencyAnalyzerCosmicsGlb *
    gemEfficiencyAnalyzerCosmicsSta *
    gemEfficiencyAnalyzerCosmicsTracker
)
