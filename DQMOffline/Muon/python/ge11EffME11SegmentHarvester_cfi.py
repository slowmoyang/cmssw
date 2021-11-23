import FWCore.ParameterSet.Config as cms
from DQMOffline.Muon.gemEfficiencyHarvesterDefault_cfi import gemEfficiencyHarvesterDefault as _gemEfficiencyHarvesterDefault

ge11EffME11SegmentHarvester_useFiducialCut_useME11A = _gemEfficiencyHarvesterDefault.clone(
    folder="GEM/Efficiency/ME11SegmentV1_useFiducialCut_useME11A",
    doResidualSummary = False
)

ge11EffME11SegmentHarvester_useFiducialCut = _gemEfficiencyHarvesterDefault.clone(
    folder="GEM/Efficiency/ME11SegmentV1_useFiducialCut",
    doResidualSummary = False
)

ge11EffME11SegmentHarvester = _gemEfficiencyHarvesterDefault.clone(
    folder="GEM/Efficiency/ME11SegmentV1",
    doResidualSummary = False
)

ge11EffME11SegmentHarvesterV2_checkBoundsWithPropError = _gemEfficiencyHarvesterDefault.clone(
    folder="GEM/Efficiency/ME11SegmentV2_checkBoundsWithPropError",
    doResidualSummary = False
)

ge11EffME11SegmentHarvesterV2_checkBoundsWithPropError_scale2 = _gemEfficiencyHarvesterDefault.clone(
    folder="GEM/Efficiency/ME11SegmentV2_checkBoundsWithPropError_scale2",
    doResidualSummary = False
)

ge11EffME11SegmentHarvesterV2 = _gemEfficiencyHarvesterDefault.clone(
    folder="GEM/Efficiency/ME11SegmentV2",
    doResidualSummary = False
)

ge11EffME11SegmentHarvesterSeq = cms.Sequence(
    ge11EffME11SegmentHarvester_useFiducialCut_useME11A *
    ge11EffME11SegmentHarvester_useFiducialCut *
    ge11EffME11SegmentHarvester *
    ge11EffME11SegmentHarvesterV2_checkBoundsWithPropError *
    ge11EffME11SegmentHarvesterV2_checkBoundsWithPropError_scale2 *
    ge11EffME11SegmentHarvesterV2
)
