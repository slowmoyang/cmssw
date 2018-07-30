#!/usr/bin/env python
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from six.moves import xrange

from gemdrawtools import Eff1DPainter
from gemdrawtools import Directory
from gemdrawtools import convert_to_title_suffix

import ROOT
from ROOT import gROOT

import argparse
from datetime import datetime


# TODO cleanup... it's too messy..
def draw_rechits_task(task_dir, out_dir):
    """
    Args:
      task_dir: A TDirectoryFile.
      out_dir: A str. A path to output directory.
    """
    keys = [each.GetName() for each in task_dir.GetListOfKeys()]

    # Efficiency Plots
    for axis in ["eta", "phi"]:
        # FIXME find better  name..
        rechit_key = "rechit_occ_" + axis
        simhit_key = "simhit_occ_" + axis

        rechit_occ = sorted(each for each in keys if rechit_key in each)
        simhit_occ = sorted(each for each in keys if simhit_key in each)
        for passed, total in zip(rechit_occ, simhit_occ):
            name_suffix = passed.replace(rechit_key, "")
            eff_name = "eff_{}{}".format(axis, name_suffix)
            title_suffix = convert_to_title_suffix(name_suffix) 
            title = "Efficiency {}{}".format(axis, title_suffix).title()

            painter = Eff1DPainter(
                passed_name=passed,
                total_name=total,
                name=eff_name,
                task_dir=task_dir,
                out_dir=out_dir,
                title=title)





def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--in_path", default="./DQM_V0001_R000000001__Global__CMSSW_X_Y_Z__RECO.root", type=str)
    parser.add_argument("-o", "--out_dir", default="DQM_{}".format(datetime.now().strftime("%y%m%d-%H%M%S")), type=str)
    args = parser.parse_args()

    dqm_file = ROOT.TFile.Open(args.in_path, "READ")

    # Task Directories
    sim_dir = dqm_file.Get("DQMData/Run 1/MuonGEMHitsV/Run summary/GEMHitsTask")
    digi_dir = dqm_file.Get("DQMData/Run 1/MuonGEMDigisV/Run summary/GEMDigisTask")
    rec_dir = dqm_file.Get("DQMData/Run 1/MuonGEMRecHitsV/Run summary/GEMRecHitsTask")

    # Output
    out_dir = Directory(args.out_dir, True)
    out_dir.make_subdir("MuonGEMHits")
    out_dir.make_subdir("MuonGEMDigis")
    out_dir.make_subdir("MuonGEMRecHits")

    draw_rechits_task(rec_dir, out_dir.MuonGEMRecHits.path)


if __name__ == "__main__":
    gROOT.SetBatch(True)
    main()
