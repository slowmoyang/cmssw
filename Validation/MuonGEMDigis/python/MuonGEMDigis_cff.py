import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from Validation.MuonGEMHits.MuonGEMCommonParameters_cfi import GEMValidationCommonParameters

gemStripValidation = DQMEDAnalyzer('GEMStripDigiValidation',
  GEMValidationCommonParameters,
  folder = cms.string("MuonGEMDigisV/GEMDigisTask/Strip"),
  logCategory = cms.string("GEMStripDigiValidation"),
  detailPlot = cms.bool(True), 
  # InputTag
  stripLabel= cms.InputTag("muonGEMDigis"),
  simhitLabel = cms.InputTag('g4SimHits', "MuonGEMHits"),
  # Additional
  nStripsGE11 =  cms.untracked.int32(384),
  nStripsGE21 =  cms.untracked.int32(768),
)


gemPadValidation = DQMEDAnalyzer('GEMPadDigiValidation',
  GEMValidationCommonParameters,
  logCategory = cms.string("GEMPadDigiValidation"),
  folder = cms.string("MuonGEMDigisV/GEMDigisTask/Pad"),
  detailPlot = cms.bool(True), 
  # Input Label
  padLabel = cms.InputTag('simMuonGEMPadDigis'),
  simhitLabel = cms.InputTag('g4SimHits', "MuonGEMHits"),
)


gemCoPadValidation = DQMEDAnalyzer('GEMCoPadDigiValidation',
  GEMValidationCommonParameters,
  logCategory = cms.string("GEMCoPadDigiValidation"),
  folder = cms.string("MuonGEMDigisV/GEMDigisTask/CoPad"),
  detailPlot = cms.bool(True), 
  copadLabel = cms.InputTag('simCscTriggerPrimitiveDigis') ,
  simhitLabel = cms.InputTag('g4SimHits',"MuonGEMHits"),
  # Additional
  minBXGEM = cms.int32(-1),
  maxBXGEM = cms.int32(1),
  # FIXME BXRange = cms.vint32(-1, 1),
)


gemGeometryChecker = DQMEDAnalyzer('GEMCheckGeometry',
  detailPlot = cms.bool(False), 
)

gemDigiValidation = cms.Sequence(gemStripValidation + gemPadValidation + gemCoPadValidation + gemGeometryChecker)
 
from Configuration.Eras.Modifier_run2_GEM_2017_cff import run2_GEM_2017
run2_GEM_2017.toReplaceWith(gemDigiValidation, gemDigiValidation.copyAndExclude([gemPadValidation,gemCoPadValidation]))
