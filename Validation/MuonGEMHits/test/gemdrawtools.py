from __future__ import division
from __future__ import print_function

import ROOT
from ROOT import TCanvas
from ROOT import TEfficiency
from ROOT import gROOT
from ROOT import gStyle
from ROOT import gPad

import re
import os
import sys
import warnings


class Directory(object):
    def __init__(self, path, creation=True):
        self.path = path
        self._creation = creation
        if self._creation:
            os.makedirs(self.path)

    def make_subdir(self, name):
        path = os.path.join(self.path, name)
        setattr(self, name, Directory(path, creation=self._creation))

    def get_entries(self, full_path=True):
        entries = os.listdir(self.path)
        if full_path:
            entries = [os.path.join(self.path, each) for each in entries]
        return entries


def get_suffix_title(det_id):
    title_suffix = ""
    if det_id["re"] is not None:
        title_suffix += " Region {}".format(det_id["re"])
    if det_id["st"] is not None:
        title_suffix += " Station {}".format(det_id["st"])
    if det_id["la"] is not None:
        title_suffix += " Layer {}".format(det_id["la"])
    if det_id["ch"] is not None:
        title_suffix += " Chamber {}".format(det_id["ch"])
    return title_suffix

def _to_int(x):
    return int(x.replace("_", "-")) 

def convert_to_title_suffix(name_suffix):
    name_suffix = name_suffix.strip("_")

    det_id = {each: None for each in ["re", "st", "la", "ch"]}

    # FIXME too messy..
    if "ch" in name_suffix:
        name_suffix, chamber_id = name_suffix.strip("_").rsplit("ch")
        det_id["ch"] = _to_int(chamber_id)
    if "la" in name_suffix: 
        name_suffix, layer_id = name_suffix.strip("_").rsplit("la")
        det_id["la"] = _to_int(layer_id)
    if "st" in name_suffix: 
        name_suffix, station_id = name_suffix.strip("_").rsplit("st")
        det_id["st"] = _to_int(station_id)
    if "re" in name_suffix: 
        name_suffix, region_id = name_suffix.strip("_").rsplit("re")
        det_id["re"] = _to_int(region_id)
    title_suffix = get_suffix_title(det_id)

    return title_suffix


class BasePainter(object):
    def __init__(self,
                 task_dir,
                 out_dir,
                 **kwargs):
        """
        Args:
          task_dir:
          out_dir:
        """
        self.task_dir = task_dir
        self.out_dir = out_dir

        self.kwargs = kwargs

        self.can = TCanvas()
        self.can.cd()

        self.draw()
        self.makeup()
        self.save()

    def draw(self):
        raise NotImplementedError()

    def makeup(self):
        raise NotImplementedError()

    def _make_out_fmt(self):
        out_fmt = os.path.join(self.out_dir, self.name + ".{ext}")
        return out_fmt

    def save(self):
        out_fmt = self._make_out_fmt()
        self.can.SaveAs(out_fmt.format(ext="png"))
        self.can.SaveAs(out_fmt.format(ext="pdf"))


class BaseEffPainter(BasePainter):
    def __init__(self,
                 passed_name,
                 total_name,
                 name,
                 task_dir,
                 out_dir,
                 **kwargs):

        self.passed_name = passed_name
        self.total_name = total_name
        self.name = name

        super(BaseEffPainter, self).__init__(task_dir, out_dir, **kwargs)

    def draw(self):
        self.passed = self.task_dir.Get(self.passed_name)
        self.total = self.task_dir.Get(self.total_name)
        if not TEfficiency.CheckConsistency(self.passed, self.total):
            # TODO
            warnings.warn("inconsisntency {} {}".format(passed_name, total_name))
            return None
        self.eff = TEfficiency(self.passed, self.total)

    def makeup(self):
        raise NotImplementedError()


class Eff1DPainter(BaseEffPainter):
    def makeup(self):
        self.eff.Draw("AP")
        self.eff.SetLineWidth(2)
        self.eff.SetLineColor(ROOT.kBlue)
        self.can.SetGrid(True)

        if self.kwargs.has_key("title"):
            title = self.kwargs["title"]

        if self.kwargs.has_key("x_title"):
            x_title = self.kwargs["x_title"]
        else:
            x_title = self.passed.GetXaxis().GetTitle()

        if self.kwargs.has_key("y_title"):
            y_title = self.kwargs["y_title"]
        else:
            y_title = "eff." 

        self.eff.SetTitle("{};{};{}".format(title, x_title, y_title))
            
