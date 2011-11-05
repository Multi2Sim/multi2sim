#!/usr/bin/python

# Copyright (C) 2006 Rafael Ubal Tena
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


import sys
import math
from tempfile import NamedTemporaryFile


# Constats
TotalColors = 30
ChartColors = [
	[0.000, 0.270, 0.523],
	[0.996, 0.258, 0.055],
	[0.996, 0.824, 0.125],
	[0.340, 0.613, 0.109],
	[0.492, 0.000, 0.129],
	[0.512, 0.789, 0.996],
	[0.191, 0.250, 0.016],
	[0.680, 0.809, 0.000],
	[0.293, 0.121, 0.434],
	[0.996, 0.582, 0.055],
	[0.770, 0.000, 0.043],
	[0.000, 0.516, 0.816]
]
GrayColors = [
	[0.000, 0.000, 0.000],
	[0.111, 0.111, 0.111],
	[0.222, 0.222, 0.222],
	[0.333, 0.333, 0.333],
	[0.444, 0.444, 0.444],
	[0.556, 0.556, 0.556],
	[0.667, 0.667, 0.667],
	[0.778, 0.778, 0.778],
	[0.889, 0.889, 0.889],
	[1.000, 1.000, 1.000]
]



# Global variables

BarPlotName = "BarPlot 1.1.6"
Data = []
Title = ""
XLabel = ""
YLabel = ""
XTics = []
Key = []
KeyColors = [
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 13, 16, 19, 11, 14, 17, 12, 15, 18,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29
]
KeyColorsSpecified = False
YRange = []
YRangeLow = 0
YRangeLowSpecified = False
YRangeHigh = 0
YRangeHighSpecified = False
YRangeDiff = 0.0
BlockCount = 0
BarCount = 0
Groups = []
Shapes = []
AddBars = []
ShapeWidth = 0.8
Labels = []
Confidence = []
Rotate = 45
GroupSeparator = True
BreakBars = True
Copyright = True
Accumulative = False
Overlap = False
PatternFill = False
HMeans = False
HMeanLabel = "H. Mean"
Averages = False
AverageLabel = "Average"
NormalizeOpt = 0
IgnoreZeros = False
Worms = []   # Worms that have been drawn: [ BlockId, BarId1, BarId2 ]

Breaks = []		# Breaks that result of grouping bars
BreakCount = 0		# Number of elements in Breaks

Baselines = []		# Array of elements [X, Y, Label] to plot baselines

Color = []
PaintColor = True

PageX1 = 0
PageY1 = 0
PageX2 = 0
PageY2 = 0

# Plot parameters
PlotOrigX = 100.0
PlotOrigY = 300.0
PlotWidth = 300.0
PlotHeight = 150.0

KeyDistance = 70
YLabelDistance = 50
KeyHeight = 0
XLabelHeight = 0
GroupsHeight = 0
TitleHeight = 0

KeyRowHeight = 15.0
KeyColumns = 2

BreakWidthRatio = 0.5
BlockspcWidthRatio = 0.5
BarspcWidthRatio = 0.1

FirstYTic = 0.0
YTicsDistance = 0.0
	


def warning(msg):
	sys.stderr.write("Warning: %s\n" % msg)



def error(msg):
	sys.stderr.write("Error: %s\n" % msg)
	sys.exit(1)



def InRange(Val, Low, High):
	if Val < Low: return Low
	if Val > High: return High
	return Val



def TranslateY(y):
	a = PlotHeight / YRangeDiff
	b = - PlotHeight * YRange[0] / YRangeDiff
	return a * y + b



def GetItems(s):
	Items = []
	while True:
		s = s.strip()
		if len(s) == 0:
			break
		
		if s[0] == "\"":
			s = s[1:]
			idx = s.find("\"")
			if idx < 0: error("'%s': wrong string format" % s)
			Items.append(s[:idx])
			s = s[idx + 1:]
		
		elif s[0] == "'":
			s = s[1:]
			idx = s.find("'")
			if idx < 0: error("'%s': wrong string format" % s)
			Items.append(s[:idx])
			s = s[idx + 1:]
		
		else:
			idx = s.find(' ')
			if idx < 0:
				Items.append(s)
				break
			Items.append(s[:idx])
			s = s[idx + 1:]
	return Items



def StrToBool(key, s):
	s = s.strip().lower()
	if s == "true":
		return True
	elif s == "false":
		return False
	else:
		error("key '%s' must be 'True' or 'False'" % key)

def StrToFloat(key, s):
	try:
		result = float(s)
	except:
		error("'%s' is not a valid floating point number" % key)
	return result



def ProcessKey(key, value):
	
	global BarCount, BlockCount

	key = key.lower().strip();
	
	if key == "title":
	
		global Title
		Title = GetItems(value)
		Title = Title[0]
	
	elif key == "yrange":
	
		global YRange, YRangeDiff
		YRange = value.split()
		if len(YRange) != 2:
			error("YRange must contain 2 elements")
		for i in range(len(YRange)):
			YRange[i] = StrToFloat("YRange[%d]" % i, YRange[i])
		YRangeDiff = YRange[1] - YRange[0]
		if (YRangeDiff <= 0.0):
			error("YRange length is 0")
	
	elif key == "yrangelow":
		
		global YRangeLow, YRangeLowSpecified
		YRangeLow = StrToFloat("YRangeLow", value)
		YRangeLowSpecified = True

	elif key == "yrangehigh":
		
		global YRangeHigh, YRangeHighSpecified
		YRangeHigh = StrToFloat("YRangeHigh", value)
		YRangeHighSpecified = True
	
	elif key == "xtics":
	
		global XTics
		XTics = GetItems(value)
		
	elif key == "key":
		
		global Key
		Key = GetItems(value)
	
	elif key == "keycolors":
		
		global KeyColors
		global KeyColorsSpecified
		KeyColors = GetItems(value)
		KeyColorsSpecified = True
	
	elif key == "label":
		
		global Labels
		Label = GetItems(value)
		if len(Label) != 6:
			error("Label must contain 6 elements")
		try:
			Label[0] = int(Label[0])
			Label[1] = int(Label[1])
			Label[2] = int(Label[2])
			Label[3] = int(Label[3])
			Label[4] = float(Label[4])
		except:
			error("wrong values for label '%s'" % Label[i])
		Labels.append(Label)
		
	elif key == "xlabel":
		
		global XLabel
		XLabel = GetItems(value)
		XLabel = XLabel[0]
	
	elif key == "ylabel":
		
		global YLabel
		YLabel = GetItems(value)
		YLabel = YLabel[0]
	
	elif key == "keydistance":
		
		global KeyDistance
		KeyDistance = StrToFloat("KeyDistance", value)
	
	elif key == "ylabeldistance":
		
		global YLabelDistance
		YLabelDistance = StrToFloat("YLabelDistance", value)
		
	elif key == "keycolumns":
		
		global KeyColumns
		KeyColumns = int(value)
		if not InRange(KeyColumns, 1, 4):
			error("KeyClumns must be in range [1..4]")
	
	elif key == "plotwidth":
		
		global PlotWidth
		PlotWidth = int(value)
		if not InRange(PlotWidth, 100, 1000):
			error("PlotWidth must be in range [100..1000]")
	
	elif key == "plotheight":
		
		global PlotHeight
		PlotHeight = int(value)
		if not InRange(PlotHeight, 100, 1000):
			error("PlotHeight must be in range [100..1000]")
		
	elif key == "color":
		
		global PaintColor
		PaintColor = StrToBool(key, value)
	
	elif key == "patternfill":
		
		global PatternFill
		PatternFill = StrToBool(key, value)
		
	elif key == "group":
		
		global Groups, Breaks
		Group = GetItems(value)
		if len(Group) != 3:
			error("Group key requires 3 arguments")
		for i in range(2):
			try:
				Group[i] = int(Group[i])
			except:
				error("wrong values for group '%s'" % Group[i])
		if not Group[0] in Breaks:
			Breaks.append(Group[0])
		if not Group[1] + 1 in Breaks:
			Breaks.append(Group[1] + 1)
		Groups.append(Group)
	
	elif key == "shape":
		
		global Shapes
		Shape = GetItems(value)
		if len(Shape) != 4:
			error("Shape key requires 4 arguments")
		try:
			Shape[0] = int(Shape[0])
			Shape[1] = int(Shape[1])
			Shape[2] = int(Shape[2])
			Shape[3] = float(Shape[3])
		except:
			error("wrong values for Shape")
		if BlockCount <= Shape[0]:
			BlockCount = Shape[0] + 1
		if BarCount <= Shape[1]:
			BarCount = Shape[1] + 1
		Shapes.append(Shape)
	
	elif key == "bar":
		
		global AddBars
		Bar = GetItems(value)
		if len(Bar) != 4:
			error("'Bar' requieres 4 arguments")
		try:
			Bar[0] = int(Bar[0])
			Bar[1] = int(Bar[1])
			Bar[2] = int(Bar[2])
			Bar[3] = float(Bar[3])
		except:
			error("wrong values for 'Bar'")
		if BlockCount <= Bar[0]:
			BlockCount = Bar[0] + 1
		if BarCount <= Bar[1]:
			BarCount = Bar[1] + 1
		AddBars.append(Bar)
	
	elif key == "shapewidth":
		
		global ShapeWidth
		try:
			ShapeWidth = float(value)
		except:
			error("ShapeWidth must be a floating point value")
		if ShapeWidth < 0 or ShapeWidth > 1:
			error("ShapeWidth must lie between 0 and 1")
	
	elif key == "groupseparator":
		
		global GroupSeparator
		GroupSeparator = StrToBool(key, value)
	
	elif key == "confidence":

		global Confidence
		Interval = GetItems(value)
		if len(Interval) != 4:
			error("'Confidence' key requires 4 arguments")
		try:
			Interval[0] = int(Interval[0])
			Interval[1] = int(Interval[1])
		except:
			error("'Confidence' key: arguments 1 and 2 must be integers")
		try:
			Interval[2] = float(Interval[2])
			Interval[3] = float(Interval[3])
		except:
			error("'Confidence' key: arguments 3 and 4 must be real numbers")
		Confidence.append(Interval)
	
	elif key == "baseline":
		
		global Baselines
		Group = GetItems(value)
		if len(Group) != 3:
			error("'Baseline' key requires 3 arguments")
		Baselines.append([StrToFloat('Baseline[0]', Group[0]),
			StrToFloat('Baseline[1]', Group[1]), Group[2]])
	
	elif key == "rotate":
		
		global Rotate
		try:
			Rotate = int(value)
		except:
			error("wrong value for Rotate")
		if Rotate != InRange(Rotate, 0, 90):
			error("Rotate must have a value between 0 and 90")
	
	elif key == "breakbars":
		
		global BreakBars
		BreakBars = StrToBool(key, value)
	
	elif key == "copyright":
		
		global Copyright
		Copyright = StrToBool(key, value)
	
	elif key == "accumulative":
		
		global Accumulative
		Accumulative = StrToBool(key, value)
	
	elif key == "averages":
		
		global Averages
		Averages = StrToBool(key, value)
	
	elif key == "averagelabel":
		
		global AverageLabel
		AverageLabel = GetItems(value)[0]
	
	elif key == "hmeans":
		
		global HMeans
		HMeans = StrToBool(key, value)
	
	elif key == "hmeanlabel":
		
		global HMeanLabel
		HMeanLabel = GetItems(value)[0]
	
	elif key == "overlap":
		
		global Overlap
		Overlap = StrToBool(key, value)
	
	elif key == "breakwidthratio":
		
		global BreakWidthRatio
		BreakWidthRatio = StrToFloat(key, value)
		if BreakWidthRatio != InRange(BreakWidthRatio, 0, 10):
			error("'BreakWidthRatio' must lie between 0 and 10")
	
	elif key == "blockspcwidthratio":
		
		global BlockspcWidthRatio
		BlockspcWidthRatio = StrToFloat(key, value)
		if BlockspcWidthRatio != InRange(BlockspcWidthRatio, 0.1, 10):
			error("'BlockspcWidthRatio' must lie between 0.1 and 10")
	
	elif key == "barspcwidthratio":
		
		global BarspcWidthRatio
		BarspcWidthRatio = StrToFloat(key, value)
		if BarspcWidthRatio != InRange(BarspcWidthRatio, 0., 10):
			error("'BarspcWidthRatio' must lie between 0.1 and 10")
	
	elif key == "normalize":
		
		global NormalizeOpt
		NormalizeOpt = StrToFloat(key, value)
	
	elif key == "ignorezeros":
		
		global IgnoreZeros
		IgnoreZeros = StrToBool(key, value)
	
	else:
		error("'" + key + "': key not recognized")




def ReadFile(name):
	
	try:
		f = open(name, "r")
	except:
		error(name + ": cannot find input file");
	
	for line in f:
		
		# Do not process empty lines
		line = line.strip()
		if line == "" or line[0] == "#":
			continue
	
		# Process a pair key=value
		eqpos = line.find("=");
		if eqpos >= 0:
			ProcessKey(line[:eqpos], line[eqpos + 1:]);
			continue
		
		# A line of Data
		idata = line.split()
		for i in range(len(idata)):
			idata[i] = StrToFloat(idata[i], idata[i])
		Data.append(idata);
		
	f.close()
	
	# Check Data integrity. If it's not present, create array of 0s
	global BlockCount, BarCount
	if BlockCount == 0 or BarCount == 0:
		if Data == []: error(name + ": no Data found in file")
		BlockCount = len(Data)
		BarCount = len(Data[0])
		for idata in Data:
			if len(idata) != BarCount:
				error(name + ": not all records have the same number of elements")
	else:
		Row = []
		for i in range(BarCount):
			Row.append(0)
		for i in range(BlockCount):
			Data.append(Row)
	if (BlockCount != len(XTics) and XTics != []):
		error(name + ": # of entries in XTics does not match # of blocks")
	
	# Other integrity checks
	global BreakBars, Accumulative, PaintColor
	if HMeans and Averages:
		error("HMeans and Averages are incompatible")
	if HMeans and Accumulative:
		warning("HMeans = Accumulative = True; harmonic means will be computed over non-accumulative values")
	if Averages and Accumulative:
		warning("Averages = Accumulative = True; averages will be computed over non-accumulative values")
	if Overlap and BreakBars:
		BreakBars = False
		warning("Overlap = True, BreakBars set to False")
	if NormalizeOpt != 0 and not Accumulative:
		Accumulative = False
		warning("Normalize != 0, Accumulative set to False")
	if PatternFill and PaintColor:
		PaintColor = False
		warning("PatternFill = True, Color set to False")
	if PatternFill and BarCount > 10:
		error("Only 10 bars per block allowed with PatternFill = True")
	if not PaintColor and BarCount > 10:
		error("Only 10 bars per block allowed with Color = False")
	
	# Key Colors
	global KeyColors
	if not KeyColorsSpecified:
		if not PaintColor:
			KeyColors = [ 10, 13, 16, 19, 11, 14, 17, 12, 15, 18 ]
		if PatternFill:
			KeyColors = [ 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 ]
	for i in range(len(KeyColors)):
		try:
			KeyColors[i] = int(KeyColors[i])
		except:
			error("KeyColors must be a list of integer values")
		if (KeyColors[i] < 0 or KeyColors[i] >= 30):
			error("KeyColors integers must be between 0 and 29")
	if len(KeyColors) < BarCount:
		error("not enough elements in KeyColors")
	
	# Averages
	if Averages:
		sys.stdout.write("Average block values: ")
		Avg = []
		for j in range(0, BarCount):
			sum = 0
			count = 0
			for i in range(0, BlockCount):
				sum = sum + Data[i][j]
				if Data[i][j] != 0 or not IgnoreZeros:
					count = count + 1
			if count > 0:
				sum = sum / count
			Avg.append(sum)
			sys.stdout.write("%g " % Avg[-1])
		print

		BlockCount = BlockCount + 1
		Data.append(Avg)
		if XTics != []:
			XTics.append(AverageLabel)
	
	if HMeans:
		sys.stdout.write("Harmonic Means block values: ")
		HM = []
		for j in range(0, BarCount):
			sum = 0
			count = 0
			for i in range(0, BlockCount):
				if Data[i][j] != 0:
					count = count + 1
					sum = sum + 1.0 / Data[i][j]
			if sum != 0:
				HM.append(count / sum)
			else:
				HM.append(0)
			sys.stdout.write("%g " % HM[-1])
		print

		BlockCount = BlockCount + 1
		Data.append(HM)
		if XTics != []:
			XTics.append(HMeanLabel)
	
	# Normalize data
	if NormalizeOpt != 0:
		for i in range(BlockCount):
			total = 0.0
			for j in range(0, BarCount):
				total = total + Data[i][j]
			if total == 0:
				continue
			for j in range(0, BarCount):
				Data[i][j] = Data[i][j] * NormalizeOpt / total
	
	# If we have accumulative data, recalculate it
	if Accumulative:
		for i in range(BlockCount):
			for j in range(1, BarCount):
				Data[i][j] = Data[i][j] + Data[i][j - 1]
	
	# Throw warnings if we have overlap and data elements descend
	# in a single record (some bar would be hidden)
	if Overlap:
		for i in range(BlockCount):
			for j in range(1, BarCount):
				if Data[i][j] < Data[i][j - 1]:
					warning("record %d contains some descendent data - some bar will be hidden" % i)
					break

	# Integrity of confidence intervals
	for Interval in Confidence:
		[X, Y, Y1, Y2] = Interval
		if Y1 > Y2:
			error("<y1> and <y2> unordered in confidence interval")
		if X < 0 or X >= BlockCount:
			error("<x> out of range in confidence interval")
		if Y < 0 or Y >= BarCount:
			error("<y> out of range in confidence interval")
		if Y1 > Data[X][Y] or Y2 < Data[X][Y]:
			warning("confidence interval does not include data at %d/%d" % (X, Y))
	
	# Compute YRange if it was not specified
	ComputeYRange()

	# Compute Groups, Breaks and BreakCount
	global Breaks, BreakCount
	for Group in Groups:
		if Group[0] > Group[1] or Group[0] != InRange(Group[0], 0, BlockCount - 1) \
		or Group[1] != InRange(Group[1], 0, BlockCount - 1):
			error("wrong range for group '%s'" % Group[2])
	Breaks.sort()
	if len(Breaks) > 0 and Breaks[0] == 0:
		Breaks = Breaks[1:]
	if len(Breaks) > 0 and Breaks[-1] == BlockCount:
		Breaks = Breaks[:-1]
	BreakCount = len(Breaks)

	# Shapes
	for Shape in Shapes:
		if Shape[0] != InRange(Shape[0], 0, BlockCount - 1) or \
		Shape[1] != InRange(Shape[1], 0, BarCount - 1) or \
		Shape[2] != InRange(Shape[2], 0, 2):
			error("wrong range for shape")
	
	# AddBars
	for Bar in AddBars:
		if Bar[0] != InRange(Bar[0], 0, BlockCount - 1) or \
		Bar[1] != InRange(Bar[1], 0, BarCount - 1) or \
		Bar[2] != InRange(Bar[2], 0, TotalColors - 1):
			error("wrong range for bar")
	


# Compute YRange, FirstYTic and YTicsDistance
def ComputeYRange():
	
	# YRange
	global YRange, YRangeDiff
	YRangeSpecified = YRange != []
	if YRangeSpecified and (YRangeLowSpecified or YRangeHighSpecified):
		warning("YRange is incompatible with YRangeLow/YRangeHigh")
	if not YRangeSpecified:
		YRange = [Data[0][0], Data[0][0]]
		for i in range(BlockCount):
			for j in range(BarCount):
				YRange[0] = min(YRange[0], Data[i][j])
				YRange[1] = max(YRange[1], Data[i][j])
			for Interval in Confidence:
				for Y in [Interval[2], Interval[3]]:
					YRange[0] = min(YRange[0], Y)
					YRange[1] = max(YRange[1], Y)
		for Bar in AddBars:
			YRange[0] = min(YRange[0], Bar[3])
			YRange[1] = max(YRange[1], Bar[3])
		for Shape in Shapes:
			YRange[0] = min(YRange[0], Shape[3])
			YRange[1] = max(YRange[1], Shape[3])
		if YRangeLowSpecified:
			YRange[0] = YRangeLow
		if YRangeHighSpecified:
			YRange[1] = YRangeHigh
		if Overlap and YRange[0] >= 0.0:
			YRange[0] = 0.0
		YRangeDiff = YRange[1] - YRange[0]
		if YRangeDiff <= 0.0:
			error("YRange difference <= 0")
	
	# Calculate FirstYTic and YTicsDistance
	global FirstYTic, YTicsDistance
	NumTics = 3
	YTicsDistance = (YRange[1] - YRange[0]) / NumTics
	NDist = Normalize(YTicsDistance)
	if NDist[0] > 5.0: NDist[0] = 5.0
	elif NDist[0] > 2.0: NDist[0] = 2.0
	else: NDist[0] = 1.0
	YTicsDistance = NDist[0] * 10.0 ** NDist[1]
	FirstYTic = math.ceil(YRange[0] / YTicsDistance) * YTicsDistance

	# Adjust YRange
	if not YRangeSpecified and not YRangeLowSpecified:
		FirstYTic = math.floor(YRange[0] / YTicsDistance) * YTicsDistance
		YRange[0] = FirstYTic
		YRangeDiff = YRange[1] - YRange[0]
	if not YRangeSpecified and not YRangeHighSpecified:
		YRange[1] = math.ceil(YRange[1] / YTicsDistance) * YTicsDistance
		YRangeDiff = YRange[1] - YRange[0]


def GetBlockCoords(BlockId):
	X = (BlockId + 1) * BlockspcWidth + BlockId * BlockWidth
	for i in Breaks:
		if i <= BlockId:
			X = X + BreakWidth
	return [X, 0, X + BlockWidth, PlotHeight]


def GetBarCoords(BlockId, BarId):
	[X1, Y1, X2, Y2] = GetBlockCoords(BlockId)
	if not Overlap:
		X1 = X1 + BarId * (BarWidth + BarspcWidth)
		X2 = X1 + BarWidth
	Y1 = InRange(TranslateY(0), 0, PlotHeight)
	Y2 = InRange(TranslateY(Data[BlockId][BarId]), 0, PlotHeight)
	return [X1, Y1, X2, Y2]


def DrawBox(f, X, Y, Width, Height, Color):

	if Color >= 0 and Color < 10:
		f.write("%f %f %f C " % (ChartColors[Color][0], ChartColors[Color][1], ChartColors[Color][2]))
		f.write("%f %f %f %f FillBox " % (X, Y, Width, Height))
		f.write("stroke\n")
	elif Color >= 10 and Color < 20:
		f.write("%f %f %f C " % (GrayColors[Color - 10][0],
			GrayColors[Color - 10][1], GrayColors[Color - 10][2]))
		f.write("%f %f %f %f FillBox " % (X, Y, Width, Height))
		f.write("stroke\n")
	else:
		f.write("1 setgray %f %f %f %f FillBox 0 setgray\n" % (X, Y, Width, Height))
		f.write("%f %f %f %f PatBox%d " % (X, Y, Width, Height, Color - 20))
		f.write("stroke\n")

	

def DrawBar(f, BlockId, BarId, Color, Height):
	[X1, Y1, X2, Y2] = GetBarCoords(BlockId, BarId)
	Y2 = InRange(TranslateY(Height), 0, PlotHeight)
	DrawBox(f, PlotOrigX + X1, PlotOrigY + Y1, BarWidth, Y2 - Y1, Color);



def DrawShapes(f):

	# Set plot region as clipping path
	f.write("gsave %f %f N %f 0 V 0 %f V %f 0 V closepath clip\n" % (PlotOrigX, PlotOrigY,
		PlotWidth, PlotHeight, -PlotWidth))

	# Draw white sticks
	for Shape in Shapes:
		[BlockId, BarId, Kind, Height] = Shape
		[X1, Y1, X2, Y2] = GetBarCoords(BlockId, BarId)
		Y2 = TranslateY(Height)
		Width = (X2 - X1) * ShapeWidth
		Center = PlotOrigX + X1 + (X2 - X1) / 2
		f.write("1 setgray %f %f N 1.5 setlinewidth %f %f V closepath stroke\n" %
			(Center, PlotOrigY + Y1, 0, Y2 - Y1))

	# Draw white and black shapes, and black sticks
	for Shape in Shapes:
		[BlockId, BarId, Kind, Height] = Shape
		[X1, Y1, X2, Y2] = GetBarCoords(BlockId, BarId)
		Y2 = TranslateY(Height)
		Width = (X2 - X1) * ShapeWidth
		Center = PlotOrigX + X1 + (X2 - X1) / 2

		if Shape[2] == 0:
			f.write("1 setgray %f %f %f 0 360 arc closepath fill stroke\n" %
				(Center, PlotOrigY + Y2, Width / 2 + 0.5))
			f.write("0 setgray %f %f %f 0 360 arc closepath fill stroke\n" %
				(Center, PlotOrigY + Y2, Width / 2))
		elif Shape[2] == 1:
			f.write("1 setgray %f %f N %f %f V %f %f V closepath fill stroke\n" %
				(Center - Width / 2 - 0.5, PlotOrigY + Y2 - Width / 2 - 0.5,
				Width + 1, 0, -Width / 2 - 0.5, Width + 1))
			f.write("0 setgray %f %f N %f %f V %f %f V closepath fill stroke\n" %
				(Center - Width / 2, PlotOrigY + Y2 - Width / 2, Width, 0, -Width / 2, Width))
		elif Shape[2] == 2:
			f.write("1 setgray %f %f N %f %f V %f %f V %f %f V closepath fill stroke\n" %
				(Center - Width / 2 - 0.5, PlotOrigY + Y2 - Width / 2 - 0.5,
				Width + 1, 0, 0, Width + 1, -Width - 1, 0))
			f.write("0 setgray %f %f N %f %f V %f %f V %f %f V closepath fill stroke\n" %
				(Center - Width / 2, PlotOrigY + Y2 - Width / 2, Width, 0, 0, Width, -Width, 0))
		else:
			error("shape not implemented")

		f.write("0 setgray %f %f N 1 setlinewidth %f %f V closepath stroke\n" %
			(Center, PlotOrigY + Y1, 0, Y2 - Y1))
	
	# Restore original draw region
	f.write("grestore\n")



# Return an array of elements of type [BlockId, BarId1, BarId2], which contain the
# groups of contiguous bars exceeding plot height
def GetContiguousBars():
	Result = []
	Last = -1
	for i in range(BlockCount):
		for j in range(BarCount):
			if Data[i][j] > YRange[1] and Last == -1:
				Last = j
			if Data[i][j] <= YRange[1] and Last != -1:
				Result.append([i, Last, j - 1])
				Last = -1
		if Last != -1:
			Result.append([i, Last, BarCount - 1])
			Last = -1
	return Result


def DrawWorm(f, BlockId, BarId1, BarId2, LeftData, RightData):

	global Worms
	
	Coords1 = GetBarCoords(BlockId, BarId1)
	Coords2 = GetBarCoords(BlockId, BarId2)
	X1 = Coords1[0]
	X2 = Coords2[2]
	Y1 = InRange(TranslateY(0), 0, PlotHeight)
	Y2 = InRange(TranslateY(LeftData), 0, PlotHeight)
	
	BreakY1 = (Y2 - Y1) * 0.8
	BreakY2 = (Y2 - Y1) * 0.85
	Left = -2
	Width = X2 - X1 + 4
	WormHeight = 3
	
	# Save and set current clipping path
	ClipWidth = X2 - X1 + BarspcWidth
	f.write("gsave\n")
	f.write("%f %f N " % (PlotOrigX + X1 - BarspcWidth/2, PlotOrigY + Y1))
	f.write("%f %f V %f %f V %f %f V " % (ClipWidth, 0, 0, Y2 - Y1, -ClipWidth, 0))
	f.write("closepath clip\n")
		
	# Central white zone
	f.write("%f %f N 1 setgray " % (PlotOrigX + X1 + Left, PlotOrigY + Y1 + BreakY1))
	f.write("%f %f %f %f %f %f rcurveto " % (Width * 0.33, -WormHeight, Width * 0.66, \
		WormHeight, Width, 0))
	f.write("%f %f V " % (0, BreakY2 - BreakY1))
	f.write("%f %f %f %f %f %f rcurveto " % (-Width * 0.33, WormHeight, -Width * 0.66, \
		-WormHeight, -Width, 0))
	f.write("closepath fill stroke\n")
	
	# Restore previous clipping path
	f.write("grestore\n")
		
	# Lower worm
	f.write("LTthick 0 setgray %f %f N " % (PlotOrigX + X1 + Left, PlotOrigY + Y1 + BreakY1))
	f.write("%f %f %f %f %f %f rcurveto " % (Width * 0.33, -WormHeight, Width * 0.66, \
		WormHeight, Width, 0))
	f.write("stroke LTdef\n")
	
	# Upper worm
	f.write("LTthick 0 setgray %f %f N " % (PlotOrigX + X1 + Left, PlotOrigY + Y1 + BreakY2))
	f.write("%f %f %f %f %f %f rcurveto " % (Width * 0.33, -WormHeight, Width * 0.66, \
		WormHeight, Width, 0))
	f.write("stroke LTdef\n")
	
	# Text labels
	Left = Right = False
	if BarId1 != BarId2:
		Left = Right = True
	else:
		First = True
		for Worm in Worms:
			if Worm[0] == BlockId:
				First = False
				break
		Left = First
		Right = not First
	if Left:
		f.write("Ts %f %f M (%g) Rshow stroke\n" % (PlotOrigX + X1 - 2, \
			PlotOrigY + PlotHeight - 8, LeftData))
	if Right:
		f.write("Ts %f %f M (%g) show stroke\n" % (PlotOrigX + X2 + 1, \
			PlotOrigY + PlotHeight - 8, RightData))

	# Record drawn worm
	Worms.append([BlockId, BarId1, BarId2])


def DrawWorms(f):
	ContiguousBars = GetContiguousBars()
	for [BlockId, BarId1, BarId2] in ContiguousBars:
		DrawWorm(f, BlockId, BarId1, BarId2, \
			Data[BlockId][BarId1], Data[BlockId][BarId2])


def WriteXTic(f, BlockId):
	if BlockId >= len(XTics) or XTics[BlockId] == "":
		return
	
	[X1, Y1, X2, Y2] = GetBlockCoords(BlockId)
	X = (X2 - X1) / 2 + X1
	f.write("gsave Tn 0 setgray %f %f N %d rotate\n" % (PlotOrigX + X, PlotOrigY - 10, Rotate))
	if Rotate == 0:
		f.write("(%s) stringwidth 3 add /y exch def 2 div /x exch def\n" % (XTics[BlockId]))
	else:
		f.write("(%s) stringwidth /y exch def /x exch def\n" % (XTics[BlockId]))
	f.write("x neg y neg R\n")
	f.write("(%s) show stroke grestore\n" % (XTics[BlockId]))



def Normalize(x):
	Exp = 0
	if x == 0.0:
		return [0.0, 0.0]
	while x >= 10.0:
		x = x / 10.0
		Exp = Exp + 1
	while x < 1.0:
		x = x * 10.0
		Exp = Exp - 1
	return [x, Exp]


def WriteYTics(f):

	# Tics
	f.write("gsave LTdef2\n")
	Y = FirstYTic
	if TranslateY(Y) < 1.0:
		Y = Y + YTicsDistance
	while Y < YRange[1]:
		PhysY = TranslateY(Y)
		f.write("newpath 0 0 0 C %f %f M " % (PlotOrigX - 2, PlotOrigY + PhysY))
		f.write("%f %f V stroke\n" % (7, 0))
		Y = Y + YTicsDistance
	f.write("grestore\n")
		
	# Grid
	f.write("gsave [3 5] 6 setdash\n")
	Y = FirstYTic
	if TranslateY(Y) < 1.0:
		Y = Y + YTicsDistance
	ZeroY = TranslateY(0)
	while Y < YRange[1]:
		PhysY = TranslateY(Y)
		if abs(PhysY - ZeroY) > 1:
			f.write("newpath 0 0 0 C %f %f M\n" % (PlotOrigX, PlotOrigY + PhysY))
			f.write("%f %f V stroke\n" % (PlotWidth, 0))
		Y = Y + YTicsDistance
	f.write("grestore\n")
	
	# Numbers
	Y = FirstYTic
	while Y <= YRange[1]:
		PhysY = TranslateY(Y)
		f.write("Tn %f %f M 0 setgray\n" % (PlotOrigX - 5, PlotOrigY + PhysY - 4))
		f.write("(%g) Rshow stroke\n" % Y)
		Y = Y + YTicsDistance



def DrawKey(f):
	
	global KeyHeight
	if Key == []:
		KeyHeight = 0
		return
	KeyRows = int(math.ceil(float(BarCount) / KeyColumns))
	KeyHeight = KeyRows * KeyRowHeight + 10
	KeyOrigY = PlotOrigY - KeyDistance - XLabelHeight - 15
	KeyOrigX = PlotOrigX + 10
	KeyWidth = PlotWidth - 20
	
	for i in range(len(Key)):
		row = i % KeyRows
		col = i / KeyRows
		square = 10.0
		X = KeyWidth * col / KeyColumns
		Y = -KeyRowHeight * row
		
		f.write("Tn %f %f M 0 setgray " % (KeyOrigX + X + square * 1.5, KeyOrigY + Y + 1))
		f.write("(%s) show\n" % Key[i])
		DrawBox(f, KeyOrigX + X - square, KeyOrigY + Y, square * 2, square, KeyColors[i])
		


def DrawGroups(f):
	
	# Print Breaks
	if GroupSeparator:
		for BlockId in Breaks:
			X1 = GetBlockCoords(BlockId - 1)[2]
			X2 = GetBlockCoords(BlockId)[0]
			X = (X2 - X1) / 2 + X1
			f.write("0 setgray %f %f M %f %f V stroke\n" % (PlotOrigX + X, PlotOrigY, 0, PlotHeight))
	
	# Update GroupsHeight
	if Groups != []:
		global GroupsHeight
		GroupsHeight = 20
	
	# Print Groups
	for Group in Groups:
		[Left, Right, Name] = Group
		
		X1 = GetBlockCoords(Left)[0] - BlockspcWidth / 2
		X2 = GetBlockCoords(Right)[2] + BlockspcWidth  / 2
		Width = (X2 - X1 + 1)
		HWidth = Width / 2
		
		# Group label
		f.write("Tn %f %f M 0 setgray\n" % (PlotOrigX + X1 + HWidth, PlotOrigY + PlotHeight + 5))
		f.write("(%s) Cshow stroke\n" % Name)


def DrawConfidence(f):
	
	# Set plot region as clipping path
	f.write("gsave %f %f N %f 0 V 0 %f V %f 0 V closepath clip\n" % (PlotOrigX, PlotOrigY,
		PlotWidth, PlotHeight, -PlotWidth))
	
	# Confidence intervals
	for Interval in Confidence:
		[X1, Y1, X2, Y2] = GetBarCoords(Interval[0], Interval[1])
		X = (X2 + X1) / 2
		Y1 = TranslateY(Interval[2])
		Y2 = TranslateY(Interval[3])
		f.write("newpath LTthick 1 1 1 C %f %f M %f %f V stroke\n" % (PlotOrigX + X, PlotOrigY + Y1, 0, Y2 - Y1))
		f.write("LTdef4 1 1 1 C %f %f M %f %f V stroke\n" % (PlotOrigX + (X + X1) / 2, PlotOrigY + Y1, BarWidth / 2, 0))
		f.write("%f %f M %f %f V stroke\n" % (PlotOrigX + (X + X1) / 2, PlotOrigY + Y2, BarWidth / 2, 0))
		f.write("LTdef2 %f %f M %f %f V stroke\n" % (PlotOrigX + X, PlotOrigY + Y1, 0, Y2 - Y1))
		f.write("LTdef2 %f %f M %f %f V stroke\n" % (PlotOrigX + (X + X1) / 2, PlotOrigY + Y1, BarWidth / 2, 0))
		f.write("%f %f M %f %f V stroke\n" % (PlotOrigX + (X + X1) / 2, PlotOrigY + Y2, BarWidth / 2, 0))
	
	# Restore clipping path
	f.write("grestore\n");


def DrawLabels(f):

	# YLabel
	if YLabel != "":
		f.write("Tn %f %f N " % (PlotOrigX - YLabelDistance, PlotOrigY + PlotHeight / 2))
		f.write("gsave 90 rotate (%s) Cshow grestore stroke\n" % YLabel)
	
	# XLabel
	if XLabel != "":
		global XLabelHeight
		XLabelHeight = 20
		f.write("Tn %f %f N " % (PlotOrigX + PlotWidth / 2, \
			PlotOrigY - KeyDistance - XLabelHeight + 8))
		f.write("(%s) Cshow stroke\n" % XLabel)


def DrawTitle(f):
	if Title == "":
		return
	global TitleHeight
	TitleHeight = 13
	f.write("0 0 0 C Tb %f %f N " % (PlotOrigX + PlotWidth / 2, PlotOrigY + PlotHeight + GroupsHeight + 8))
	f.write("(%s) Cshow stroke\n" % Title)


def DrawBaselines(f):
	for Baseline in Baselines:
		X = InRange(Baseline[0], -PlotWidth / 2, PlotWidth / 2);
		Y = TranslateY(Baseline[1])
		Label = Baseline[2]
		if Y != InRange(Y, 0, PlotHeight):
			continue
		
		f.write("gsave newpath LTthick\n")
		f.write("0 0 0 C %f %f M %f %f V stroke\n" % (PlotOrigX, PlotOrigY + Y, PlotWidth, 0))
		f.write("newpath 1 1 1 C Ts (%s) stringwidth pop 6 add /ww exch def "
			"%f ww 2 div sub %f ww 10 FillBox\n" %
			(Label, PlotOrigX + PlotWidth / 2 + X, PlotOrigY + Y - 5))
		f.write("0 0 0 C %f %f M Ts (%s) Cshow " % (PlotOrigX + PlotWidth / 2 + X,
			PlotOrigY + Y - 3, Label))
		f.write("stroke grestore\n")


def DrawTextLabels(f):
	for Label in Labels:
		(x, y, dx, dy, ldy, str) = Label
		Data[x][y] += ldy
		[X1, Y1, X2, Y2] = GetBarCoords(x, y)
		Data[x][y] -= ldy
		x = PlotOrigX + (X1 + X2) / 2 + dx
		y = PlotOrigY + Y2 + dy
		f.write("Tn %f %f N " % (x, y))
		f.write("gsave (%s) Cshow grestore stroke\n" % str)


def WriteHeader(f):
	f.write("""
/M {moveto} bind def
/L {lineto} bind def
/R {rmoveto} bind def
/V {rlineto} bind def
/N {newpath M} bind def
/C {setrgbcolor} bind def

/LTdef {0.25 setlinewidth 0 setgray} bind def
/LTdef2 {0.5 setlinewidth 0 setgray} bind def
/LTdef3 {0.75 setlinewidth 0 setgray} bind def
/LTdef4 {1.0 setlinewidth 0 setgray} bind def
/LTthick {1.25 setlinewidth 0 setgray} bind def
/LTthick2 {1.5 setlinewidth 0 setgray} bind def
LTdef
	
/FillBox {/h exch def /w exch def /y exch def /x exch def
  gsave x y N w 0 V 0 h V w neg 0 V 0 h neg V fill stroke
  x y N 0 setgray w 0 V 0 h V w neg 0 V 0 h neg V LTdef stroke grestore} def

/FillTrap {/h exch def /w exch def /y exch def /x exch def
  gsave x 2 sub y N w 0 V 4 h V w neg 0 V -4 h neg V fill stroke
  x 2 sub y N 0 setgray w 0 V 4 h V w neg 0 V -4 h neg V LTdef stroke grestore} def

/Rshow {dup stringwidth pop neg 0 R show} def
/Cshow {dup stringwidth pop -2 div 0 R show} def
/Tt {/Times-Roman findfont 5 scalefont setfont} def
/Ts {/Helvetica findfont 8 scalefont setfont} def
/Tn {/Helvetica findfont 11 scalefont setfont} def
/Tb {/Times-Roman findfont 14 scalefont setfont} def
""")

	f.write("""
/Point {gsave newpath 0.3 0 360 arc fill stroke grestore} def

/PatBox0 {/h exch def /w exch def /y exch def /x exch def
  gsave x y N w 0 V 0 h V w neg 0 V 0 h neg V clip
  /dist 2.5 def
  0 dist w {/i exch def /shift 0 def 0 dist sqrt h {/j exch def
    x i add shift add y j add Point
    /shift dist 2 div shift sub def
  } for } for
  stroke grestore} def

/PatBox1 {/h exch def /w exch def /y exch def /x exch def
  gsave x y N w 0 V 0 h V w neg 0 V 0 h neg V clip
  /dist 5 def
  0 dist w {/i exch def /shift 0 def 0 dist sqrt h {/j exch def
    x i add shift add y j add Point
    /shift dist 2 div shift sub def
  } for } for
  stroke grestore} def

/PatBox2 {/h exch def /w exch def /y exch def /x exch def
  gsave x y N w 0 V 0 h V w neg 0 V 0 h neg V clip
  h neg 4 w {/now exch def x now add y M h h V} for
  stroke grestore} def

/PatBox3 {/h exch def /w exch def /y exch def /x exch def
  gsave x y N w 0 V 0 h V w neg 0 V 0 h neg V clip
  0 5 w h add {/now exch def x y now add M w w neg V} for
  h neg 5 w {/now exch def x now add y M h h V} for
  stroke grestore} def

/PatBox4 {/h exch def /w exch def /y exch def /x exch def
  gsave x y N w 0 V 0 h V w neg 0 V 0 h neg V clip
  0 4 w h add {/now exch def x y now add M w w neg V} for
  stroke grestore} def

/PatBox5 {/h exch def /w exch def /y exch def /x exch def
  gsave x y N w 0 V 0 h V w neg 0 V 0 h neg V clip
  0 2 h {/now exch def x y now add M w 0 V} for
  stroke grestore} def

/PatBox6 {/h exch def /w exch def /y exch def /x exch def
  gsave 0.9 setgray
  x y N w 0 V 0 h V w neg 0 V 0 h neg V fill stroke grestore} def

/PatBox7 {/h exch def /w exch def /y exch def /x exch def
  gsave 0.6 setgray
  x y N w 0 V 0 h V w neg 0 V 0 h neg V fill stroke grestore} def

/PatBox8 {/h exch def /w exch def /y exch def /x exch def
  gsave 0.3 setgray
  x y N w 0 V 0 h V w neg 0 V 0 h neg V fill stroke grestore} def

/PatBox9 {/h exch def /w exch def /y exch def /x exch def
  x y N w 0 V 0 h V w neg 0 V 0 h neg V fill stroke} def

""")


def Plot(FileName):
	
	# Open destination file and create a temporary file
	f = NamedTemporaryFile(mode='w+t')
	try:
		outf = open(FileName, "wt")
	except:
		error(FileName + ": cannot open output file")
	
	# BlockspcWidth * (BlockCount + 1) + BlockWidth * BlockCount + BreakWidth * BreakCount = PlotWidth
	global BlockWidth, BlockspcWidth, BreakWidth
	BlockWidth = PlotWidth / (BlockspcWidthRatio * (BlockCount + 1) + BlockCount + BreakWidthRatio * BreakCount)
	BlockspcWidth = BlockWidth * BlockspcWidthRatio
	BreakWidth = BlockWidth * BreakWidthRatio
	
	# BarspcWidth * (BarCount - 1) + BarWidth * BarCount = BlockWidth
	global BarWidth, BarspcWidth
	if Overlap:
		BarWidth = BlockWidth
		BarspcWidth = 0
	else:
		BarWidth = BlockWidth / (BarspcWidthRatio * (BarCount - 1) + BarCount)
		BarspcWidth = BarspcWidthRatio * BarWidth
	
	# Header
	WriteHeader(f)
	
	# Write XTics
	for i in range(BlockCount):
		WriteXTic(f, i)
	
	# YTics
	WriteYTics(f)
	
	# Draw Bars
	for Bar in AddBars:
		DrawBar(f, Bar[0], Bar[1], Bar[2], Bar[3])
		if BreakBars and Bar[3] > YRange[1]:
			DrawWorm(f, Bar[0], Bar[1], Bar[1], Bar[3], Bar[3])
	for i in range(BlockCount):
		for j in range(BarCount - 1, -1, -1):
			DrawBar(f, i, j, KeyColors[j], Data[i][j])
	
	# Plot shapes
	DrawShapes(f)
	
	# Worms
	if BreakBars:
		DrawWorms(f)
	
	# Draw Plot Region
	f.write("newpath 0 0 0 C %f %f M\n" % (PlotOrigX, PlotOrigY))
	f.write("%f %f V %f %f V\n" % (PlotWidth, 0, 0, PlotHeight));
	f.write("%f %f V %f %f V\n" % (-PlotWidth, 0, 0, -PlotHeight));
	f.write("stroke\n");
	
	# Draw an horizonal line at Y=0
	Y = TranslateY(0)
	if (Y == InRange(Y, 0, PlotHeight - 1)):
		f.write("newpath 0 0 0 C %f %f M\n" % (PlotOrigX, PlotOrigY + Y))
		f.write("%f %f V stroke\n" % (PlotWidth, 0))
	
	# Below: labels and Key
	DrawLabels(f)
	DrawKey(f)
	
	# Above: groups and title
	DrawGroups(f)
	DrawTitle(f)
	
	# Copyright
	if Copyright:
		str = "%s, by R. Ubal" % BarPlotName
		f.write("Tt %f %f N " % (PlotOrigX + PlotWidth + 5, PlotOrigY + PlotHeight / 2))
		f.write("gsave 0 0 0 C 90 rotate (%s) Cshow grestore stroke\n" % str)
	
	# Baselines
	DrawBaselines(f)

	# Confidence intervals & text labels
	DrawConfidence(f)
	DrawTextLabels(f)
	
	# Trailer
	PageX1 = PlotOrigX - YLabelDistance
	PageY1 = PlotOrigY - KeyDistance - XLabelHeight - KeyHeight
	PageX2 = PlotOrigX + PlotWidth + 20
	PageY2 = PlotOrigY + PlotHeight + GroupsHeight + TitleHeight + 5
	if YLabel != "":
		PageX1 = PageX1 - 20
	if Title != "":
		PageY2 = PageY2 + 15
	PageWidth = PageX2 - PageX1
	PageHeight = PageY2 - PageY1

	# Write header into output file
	outf.write("%!PS-Adobe-2.0 EPSF-2.0\n")
	#outf.write("%%Trailer\n")
	outf.write("%%%%BoundingBox: %d %d %d %d\n" % (PageX1, PageY1, PageX2, PageY2))
	if PageX2 - PageX1 > PageY2 - PageY1:
		outf.write("%%Orientation: Portrait\n")
	
	# Copy temp file into output file
	f.seek(0)
	for line in f:
		outf.write(line)
	f.close()
	outf.close()



# Main program

syntax = BarPlotName + """
A tool to generate bar charts with EPS output.

Syntax: barplot <datafile> <out>
      <datafile>      File with plotting Data and graph information
      <out>           Output 'eps' file


The format of <datafile> is:
	# This is a comment
	[Key1 = Value1]
	[Key2 = Value2] ...
	
	V11 V12 V13 ...
	V21 V22 V23 ...
	...

Possible pairs key-value are:

    Title = 'String'		String appearing over the plot.
    XTics = 'tic1' 'tic2'...	Names assigned to each block of bars. They are
    				located under the x-axis. There must be as much
				tics as number of data records.
    Key = 'key1' 'key2'...	String appearing in the key for each color.
    				There must be as much strings as number of
				elements in each data record.
    KeyColors = <c1> <c2> ...   Colors for the bars. 0..9=colors; 10..19=grayscale;
                                20..29=patterns.
    YRange = <MinY> <MaxY>	Starting and ending plotted y-coordinates.
    KeyColumns = <n>		Number of columns in the key
    Label = <x> <y> <dx> <dy>   Draw a label on top of the item x/y of the data
        <ldy> 'String'          shifted dx/dy pixels, and a logical vertical shift.
    XLabel = 'String'		X-axis label.
    YLabel = 'String'		Y-axis label.
    KeyDistance = <n>		Distance reserved for the XTics strings.
    YLabelDistance = <n>        Distance reserved for Y label
    PlotWidth = <n>		Width of plot region.
    PlotHeight = <n>		Height of plot region.
    Group = <x1> <x2> 'Name'	Group of blocks labelled with a specific string.
    				There can be various entries of this key.
    Shape = <group> <bar>       Draw a shape: 0=circle, 1=triangle, 2=square
      <shape> <y>
    ShapeWidth = <x>            Shape width ratio (1=bar width)
    Bar = <group> <bar>         Add a bar behind the bars in the array
      <style> <y>
    GroupSeparator =            If True, use a line to separate bars of different
        {True|False}            groups.
    Confidence = <x> <y>        Draw confidence interval y1-y2 for the item x/y in
        <y1> <y2>               the data matrix
    Baseline = <x> <y> 'Name'	Draw baseline label. <x> is the physical coordinate
    				respect to the center of the plot region.
    Color = {True|False}	Color or grayscale plot.
    PatternFill = {True|False}	Use patterns to fill bars.
    Rotate = <angle>            Ratation angle for XTics between 0 and 90.
    BreakBars = {True|False}	Enable bars break when they exceed YRange.
    Copyright = {True|False}	Print copyright.
    Accumulative = {True|False}	When True, accumulative data is represented.
    				That is, if a record contains "1 2 2", it will
				be transformed to "1 3 5" before plotting.
    Averages = {True|False}	Include a block with averages
    AverageLabel = 'String'	Label to include in the averages block
    HMeans = {True|False}	Include a block with harmonic means
    HMeanLabel = 'String'	Label to include in the harmonic means block
    IgnoreZeros = {True|False}  Ignore zero values to compute averages or means
    Overlap = {True|False}	Overlap bars or not. If True, each element of
    				a specific record should be greater than the
				previous one, or Accumulative should be True.
    BreakWidthRatio = 0.5	Aspect ratio of the break width respect to the
    				block width. A break is placed to separate
				elements of different block groups.
    BlockspcWidthRatio = 0.5	Aspect ratio of the block space width respect
    				to the block width. The block space is the
				space between bar blocks.
    BarspcWidthRatio = 0.1	Aspect ratio of the bar space width respect
    				to the block width. The bar space is the space
				between bars of the same block.
    Normalize = 0		Normalize record components to a specific
    				value. 0 means no normalization
"""
if len(sys.argv) != 3:
	print syntax
	sys.exit(1)

ReadFile(sys.argv[1])
Plot(sys.argv[2])
