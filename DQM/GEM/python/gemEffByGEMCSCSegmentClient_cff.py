from DQM.GEM.gemEffByGEMCSCSegmentClient_cfi import *
from DQM.GEM.gemEffByGEMCSCSegmentSource_cff import gemEffByGEMCSCSegmentSource as _gemEffByGEMCSCSegmentSource

gemEffByGEMCSCSegmentClient.folder = _gemEffByGEMCSCSegmentSource.folder.value()
