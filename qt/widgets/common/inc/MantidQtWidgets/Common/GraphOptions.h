// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

/**
 * This file contains declarations of options such as scale types that are
 * shared between the colormaps and instrument window
 */
namespace GraphOptions {

/**
 * Scale type enumeration
 */
enum ScaleType { Linear = 0, Log10, Power };

/**
 * Axis choice
 */
enum Axis { Left, Right, Bottom, Top };

/**
 * Graph curve type
 */
enum CurveType {
  Unspecified = -1,
  Line,
  Scatter,
  LineSymbols,
  VerticalBars,
  Area,
  Pie,
  VerticalDropLines,
  Spline,
  HorizontalSteps,
  Histogram,
  HorizontalBars,
  VectXYXY,
  ErrorBars,
  Box,
  VectXYAM,
  VerticalSteps,
  ColorMap,
  GrayScale,
  ColorMapContour,
  Contour,
  Function,
  ImagePlot,
  User
};
} // namespace GraphOptions
