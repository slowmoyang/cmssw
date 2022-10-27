import FWCore.ParameterSet.Config as cms

from DQM.GEM.gemEffByGEMCSCSegmentClient_cfi import *
from DQM.GEM.gemEffByGEMCSCSegmentSource_cff import gemEffByGEMCSCSegmentSourceNoChErr as _gemEffByGEMCSCSegmentSourceNoChErr

gemEffByGEMCSCSegmentClientNoChErr = gemEffByGEMCSCSegmentClient.clone(
    folder = _gemEffByGEMCSCSegmentSourceNoChErr.folder.value()
)
