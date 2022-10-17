import FWCore.ParameterSet.Config as cms

from DQM.GEM.gemEffByGEMCSCSegmentSource_cfi import *

gemEffByGEMCSCSegmentSourceNoChErr = gemEffByGEMCSCSegmentSource.clone(
    checkChamberStatus = True,
    folder = "GEM/Efficiency/GEMCSCSegment_NoChamberError"
)
