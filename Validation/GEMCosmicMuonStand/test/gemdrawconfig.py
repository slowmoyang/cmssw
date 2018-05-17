from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

# Global Constants
NUM_ROW = 5 # the number of rows in the stand
NUM_COLUMN = 3 # the number of columns in the stand
NUM_SUPERCHAMBER = 15 # the number of superchambers in the stand
NUM_LAYER = 2 # the number of layers (chambers) in each superchamber
NUM_ROLL = 8 # the number of rolls in each chamber
NUM_VFAT = 3 # the number of VFATs in each roll
NUM_STRIP = 384 # the number of readout strips per eta-segment

OPENING_ANGLE = 0.177151 # 10.15 * (math.pi / 180)
ONE_DEGREE = 0.0174533;

MIN_CHAMBER_ID = 1
MAX_CHAMBER_ID = 29
MIN_LAYER_ID = 1
MAX_LAYER_ID = 2
MIN_ROLL_ID = 1 # iEta
MAX_ROLL_ID = 8
MIN_VFAT_ID = 1
MAX_VFAT_ID = 3
