import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from Validation.MuonGEMHits.MuonGEMCommonParameters_cfi import GEMValidationCommonParameters

gemRecHitsValidation = DQMEDAnalyzer('GEMRecHitsValidation',
    GEMValidationCommonParameters,
    folder = cms.string("MuonGEMRecHitsV/GEMRecHitsTask"),
    logCategory = cms.string("GEMRecHitsValidation"),
    detailPlot = cms.bool(True),
    # input tag
    simhitLabel = cms.InputTag('g4SimHits',"MuonGEMHits"),
    rechitLabel = cms.InputTag('gemRecHits'),

)

gemLocalRecoValidation = cms.Sequence(gemRecHitsValidation)
