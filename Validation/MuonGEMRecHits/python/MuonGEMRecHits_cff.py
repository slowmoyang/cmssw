import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from RecoMuon.TrackingTools.MuonServiceProxy_cff import *
from Validation.MuonGEMHits.MuonGEMCommonParameters_cfi import GEMValidationCommonParameters

gemRecHitsValidation = DQMEDAnalyzer('GEMRecHitsValidation',
    MuonServiceProxy,
    GEMValidationCommonParameters,
    folder = cms.string("MuonGEMRecHitsV/GEMRecHitsTask"),
    logCategory=cms.string("GEMRecHitsValidation"),
    detailPlot = cms.bool(True),
    # Input tags
    muonLabel=cms.InputTag("muons"),
    # additional
    muonpTCut=cms.double(10.0)
)

gemLocalRecoValidation = cms.Sequence(gemRecHitsValidation)
