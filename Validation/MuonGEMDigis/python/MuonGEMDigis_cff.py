import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from Validation.MuonHits.muonSimHitMatcherPSet import muonSimHitMatcherPSet
from Validation.MuonGEMDigis.muonGEMDigiPSet import muonGEMDigiPSet

commonPSet = cms.PSet(
  simTrack = muonSimHitMatcherPSet.simTrack,
  simVertex = muonSimHitMatcherPSet.simVertex,
  # st1, st2 of xbin, st1, st2 of ybin
  nBinGlobalZR = cms.untracked.vdouble(200,200,150,250),
  # st1 xmin xmax, st2 xmin xmax, st1 ymin ymax, st2 ymin ymax
  RangeGlobalZR = cms.untracked.vdouble(564,574,792,802,110,290,120,390),
  nBinGlobalXY = cms.untracked.int32(360),
  outputFile = cms.string(''),
  detailPlot = cms.bool(False),
)

gemStripValidation = DQMEDAnalyzer('GEMStripDigiValidation',
  commonPSet,
  gemStripDigi = muonGEMDigiPSet.gemStripDigi,
)
gemUnpackedStripValidation = DQMEDAnalyzer('GEMStripDigiValidation',
  commonPSet,
  gemStripDigi = muonGEMDigiPSet.gemUnpackedStripDigi,
)
gemPadValidation = DQMEDAnalyzer('GEMPadDigiValidation',
  commonPSet,
  gemPadDigi = muonGEMDigiPSet.gemPadDigi,
)
gemClusterValidation = DQMEDAnalyzer('GEMPadDigiClusterValidation',
  commonPSet,
  gemPadCluster = muonGEMDigiPSet.gemPadCluster,
)
gemCoPadValidation = DQMEDAnalyzer('GEMCoPadDigiValidation',
  commonPSet,
  gemCoPadDigi = muonGEMDigiPSet.gemCoPadDigi,
)

gemDigiTrackValidation = DQMEDAnalyzer('GEMDigiTrackMatch',
  simTrack = muonSimHitMatcherPSet.simTrack,
  simVertex = muonSimHitMatcherPSet.simVertex,
  gemSimHit = muonSimHitMatcherPSet.gemSimHit,
  gemStripDigi = muonGEMDigiPSet.gemStripDigi,
  gemPadDigi = muonGEMDigiPSet.gemPadDigi,
  gemPadCluster = muonGEMDigiPSet.gemPadCluster,
  gemCoPadDigi = muonGEMDigiPSet.gemCoPadDigi,
  detailPlot = cms.bool(False),
)

gemGeometryChecker = DQMEDAnalyzer('GEMCheckGeometry',
  detailPlot = cms.bool(False),
)

gemDigiValidation = cms.Sequence( gemUnpackedStripValidation+
                                  gemPadValidation+
                                  gemClusterValidation+
                                  gemCoPadValidation+
                                  gemDigiTrackValidation+
                                  gemGeometryChecker)

from Configuration.Eras.Modifier_run2_GEM_2017_cff import run2_GEM_2017
run2_GEM_2017.toReplaceWith(gemDigiValidation, gemDigiValidation.copyAndExclude([gemPadValidation,gemClusterValidation,gemCoPadValidation]))
