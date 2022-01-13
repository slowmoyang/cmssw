import FWCore.ParameterSet.Config as cms
from RecoLocalMuon.GEMSegment.gemSegments_cfi import gemSegments

gemSegmentsBkgMitigation = gemSegments.clone(
    ge0_name = "GE0SegAlgoRUWithBkgMitigation",
    ge0_pset = gemSegments.ge0_pset.clone(
        modelPath = cms.FileInPath("RecoLocalMuon/GEMSegment/data/BkgMitigationTransformer_Opset-13.onnx"),
        inputNames = cms.untracked.vstring("input_data", "input_mask"),
        outputNames = cms.untracked.vstring("output_sigmoid"),
        numFeatures = cms.untracked.uint32(4),
        threshold = cms.untracked.double(0.5),
    ),
)

gemSegmentsTask = cms.Task(
    gemSegments,
    gemSegmentsBkgMitigation
)
