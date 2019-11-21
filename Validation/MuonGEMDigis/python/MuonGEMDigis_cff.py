import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from Validation.MuonGEMHits.MuonGEMCommonParameters_cfi import GEMValidationCommonParameters

gemStripValidation = DQMEDAnalyzer('GEMStripDigiValidation',
  GEMValidationCommonParameters,
  folder = cms.string("MuonGEMDigisV/GEMDigisTask/Strip"),
  logCategory = cms.string("GEMStripDigiValidation"),
  detailPlot = cms.bool(True), 
  # input tag
  stripLabel= cms.InputTag("muonGEMDigis"),
  simhitLabel = cms.InputTag('g4SimHits', "MuonGEMHits"),
)


gemPadValidation = DQMEDAnalyzer('GEMPadDigiValidation',
  GEMValidationCommonParameters,
  folder = cms.string("MuonGEMDigisV/GEMDigisTask/Pad"),
  logCategory = cms.string("GEMPadDigiValidation"),
  detailPlot = cms.bool(True), 
  # input tag
  padLabel = cms.InputTag('simMuonGEMPadDigis'),
  simhitLabel = cms.InputTag('g4SimHits', "MuonGEMHits"),
)


gemCoPadValidation = DQMEDAnalyzer('GEMCoPadDigiValidation',
  GEMValidationCommonParameters,
  folder = cms.string("MuonGEMDigisV/GEMDigisTask/CoPad"),
  logCategory = cms.string("GEMCoPadDigiValidation"),
  detailPlot = cms.bool(True), 
  # input tag
  copadLabel = cms.InputTag('simCscTriggerPrimitiveDigis') ,
  simhitLabel = cms.InputTag('g4SimHits',"MuonGEMHits"),
  # additional
  GEMBXRange = cms.untracked.vint32(-1, 1),
)


gemGeometryChecker = DQMEDAnalyzer('GEMCheckGeometry',
  detailPlot = cms.bool(False), 
)

gemDigiValidation = cms.Sequence(gemStripValidation + gemPadValidation + gemCoPadValidation + gemGeometryChecker)
 
from Configuration.Eras.Modifier_run2_GEM_2017_cff import run2_GEM_2017
run2_GEM_2017.toReplaceWith(gemDigiValidation, gemDigiValidation.copyAndExclude([gemPadValidation,gemCoPadValidation]))
