import FWCore.ParameterSet.Config as cms

from DQM.GEM.gemEffByGEMCSCSegmentSource_cff import *
from DQM.GEM.gemEffByGEMCSCSegmentClient_cff import *

gemEffByGEMCSCSegment = cms.Sequence(
    gemEffByGEMCSCSegmentSource *
    gemEffByGEMCSCSegmentClient
)
