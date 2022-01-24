import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from DQM.GEM.gemEfficiencyAnalyzerDefault_cfi import gemEfficiencyAnalyzerDefault
from RecoMuon.TrackingTools.MuonServiceProxy_cff import *
from TrackingTools.TransientTrack.TransientTrackBuilder_cfi import *

gemEfficiencyAnalyzer = gemEfficiencyAnalyzerDefault.clone(
    ServiceParameters = MuonServiceProxy.ServiceParameters.clone(),
)

from Configuration.Eras.Modifier_phase2_GEM_cff import phase2_GEM
phase2_GEM.toModify(
    gemEfficiencyAnalyzer,
    muonAbsEtaMax = 3.0, # FIXME need to be tuned
    etaNbins = 15,
    etaUp = 3.0
)
