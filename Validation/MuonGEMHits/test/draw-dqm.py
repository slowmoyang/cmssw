from __future__ import divison
from __future__ import print_function

import sys
import argparse
import ROOT

if sys.version_info.major == 2:
    from pathlib2 import Path
elif sys.version_info[:2] >= (3, 4):
    from pathlib import Path
else:
    raise ImportError


def draw(me, filesys_dir):
    name = me.GetName()

    canvas_name = 'canvas_' + name
    if 'occ_det' in name:
         canvas = ROOT.TCanvas(canvas_name, canvas_name, 2400, 1200)
    else:
        canvas = ROOT.TCanvas(canvas_name, canvas_name, 1200, 800)
    canvas.cd()

    me_class_name = me.ClassName()
    if me_class_name == 'TH1F':
        me.SetLineColor(ROOT.kBlue)
        me.SetLineWidth(2)
        me.Draw('hist')
    elif me_class_name == 'TProfile':
        me.SetLineColor(ROOT.kRed)
        me.SetLineWidth(2)
        me.Draw()
    elif me_class_name == 'TH2F':
        if 'polar' in name:
            max_r = me.GetYaxis().GetXmax()
            canvas.DrawFrame(-max_r, -max_r, max_r, max_r)
            me.DrawCopy('same colz pol')
            latex = ROOT.TLatex()
            latex.DrawLatexNDC(0.15, 0.92, me.GetTitle())
        elif 'occ_det' in name:
            canvas.SetBottomMargin(0.22)
            me.GetXaxis().SetTitleOffset(1.4)
            me.GetXaxis().LabelsOption('v')
            me.GetYaxis().SetTitleOffset(0.4)
            me.Draw('colz')
        else:
            me.Draw('colz')
    else:
        raise TypeError

    path = str(filesys_dir.joinpath(name + '.png'))
    canvas.SaveAs(path)


def draw_all(root_dir, filesys_dir):
    filesys_dir.mkdir()
    for key in root_dir.GetListOfKeys():
        obj = root_dir.Get(key.GetName())
        class_name = key.GetClassName()

        if isinstance(obj, ROOT.TDirectoryFile):
            subdir = filesys_dir.joinpath(obj.GetName())
            draw_all(obj, subdir)
        else:
            draw(obj, filesys_dir)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input-path')
    args = parser.parse_args()

    root_file = ROOT.TFile(args.input_path)
    root_dir = root_file.Get('DQMData/Run 1')

    filesys_dir = Path('./DQM')
    if filesys_dir.exists():
        raise IOError
    filesys_dir.mkdir()

    draw_all(root_dir.Get('MuonGEMHitsV/Run summary/GEMHitsTask'),
             filesys_dir.joinpath('MuonGEMHits'))

    draw_all(root_dir.Get('MuonGEMDigisV/Run summary/GEMDigisTask'),
             filesys_dir.joinpath('MuonGEMDigis'))

    draw_all(root_dir.Get('MuonGEMRecHitsV/Run summary/GEMRecHitsTask'),
             filesys_dir.joinpath('MuonGEMRecHits'))


if __name__ == '__main__':
    ROOT.gROOT.SetBatch(True)
    main()
