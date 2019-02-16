import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer

from Validation.MuonGEMHits.MuonGEMCommonParameters_cfi import GEMValidationCommonParameters

gemSimHitValidation = DQMEDAnalyzer('GEMHitsValidation',
    GEMValidationCommonParameters,
    folder=cms.string("MuonGEMHitsV/GEMHitsTask"),
    logCategory=cms.string("GEMHitsValidation"),
    detailPlot = cms.bool(True), 
    # InputTag
    simhitLabel = cms.InputTag('g4SimHits', "MuonGEMHits"),
    # additional
    TOFRange = cms.untracked.vdouble(18, 22, # GEM11
                                     26, 30), # GE21
)


gemSimValidation = cms.Sequence(gemSimHitValidation)
