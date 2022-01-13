import FWCore.ParameterSet.Config as cms

from RecoLocalMuon.GEMRecHit.gemRecHits_cfi import *
from RecoLocalMuon.GEMSegment.gemSegments_cff import *

gemLocalRecoTask = cms.Task(gemRecHits,gemSegmentsTask)
gemLocalReco = cms.Sequence(gemLocalRecoTask)
