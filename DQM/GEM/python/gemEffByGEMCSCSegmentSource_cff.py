from DQM.GEM.gemEffByGEMCSCSegmentSource_cfi import *

from Configuration.Eras.Modifier_run3_GEM_cff import run3_GEM
run3_GEM.toModify(gemEffByGEMCSCSegmentSource, monitorGE21=True)
