// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidTestHelpers/WorkspaceCreationHelper.h"
#include <cxxtest/TestSuite.h>

#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/Axis.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAlgorithms/ConvertSingleSpectrumLambdaToQ.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidKernel/UnitFactory.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace Mantid::Algorithms;
using namespace Mantid::DataObjects;

namespace {
/// Create a single-spectrum lambda workspace and register in the ADS
void setUpSingleSpectrumLambdaWs(std::string &inputWs) {
  auto dataX = {256, 128, 194};
  auto dataY = {11, 10, 9};
  auto dataE = {10, 9, 8};
  auto space2D =
      createWorkspace<Workspace2D>(dataX, dataY, dataE, 1, "Wavelength");
  // space2D->getAxis(0)->unit() = UnitFactory::Instance().create("Wavelength");
  AnalysisDataService::Instance().addOrReplace(inputWs, space2D);
}

/// Create a multi-spectra lambda workspace and register in the ADS
void setUpMultiSpectraLambdaWs(std::string &inputWs) {
  auto dataX = {256, 256, 256};
  auto dataY = {11, 11, 11};
  auto dataE = {10, 10, 10};
  auto space2D =
      createWorkspace<Workspace2D>(dataX, dataY, dataE, 3, "Wavelength");
  AnalysisDataService::Instance().addOrReplace(inputWs, space2D);
}

/// Create a single spectrum momentum workspace and register in the ADS
void setUpSingleSpectrumMomentumWs(std::string &inputWs) {
  auto dataX = {256, 128, 194};
  auto dataY = {11, 10, 9};
  auto dataE = {10, 9, 8};
  auto space2D =
      createWorkspace<Workspace2D>(dataX, dataY, dataE, "MomentumTransfer");
  AnalysisDataService::Instance().addOrReplace(inputWs, space2D);
}
} // namespace

class ConvertSingleSpectrumLambdaToQTest : public CxxTest::TestSuite {
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static ConvertSingleSpectrumLambdaToQTest *createSuite() {
    return new ConvertSingleSpectrumLambdaToQTest();
  }
  static void destroySuite(ConvertSingleSpectrumLambdaToQTest *suite) {
    delete suite;
  }

  void setUp() override { inputWs = "testWorkspace"; }
  void testInit() {
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT(alg.isInitialized());
  }

  /// Tests the execution of the algorithm with a single-spectrum lambda
  /// workspace
  void testExecSingleSpectrumLambdaInput() {
    setUpSingleSpectrumLambdaWs(inputWs);

    ConvertSingleSpectrumLambdaToQ ConvertSingleSpectrumLambdaToQ;
    TS_ASSERT_THROWS_NOTHING(ConvertSingleSpectrumLambdaToQ.initialize());
    TS_ASSERT(ConvertSingleSpectrumLambdaToQ.isInitialized());
    TS_ASSERT_THROWS_NOTHING(ConvertSingleSpectrumLambdaToQ.setPropertyValue(
        "InputWorkspace", inputWs));
    TS_ASSERT_THROWS_NOTHING(ConvertSingleSpectrumLambdaToQ.setPropertyValue(
        "OutputWorkspace", "outputWs"));
    TS_ASSERT_THROWS_NOTHING(
        ConvertSingleSpectrumLambdaToQ.setPropertyValue("ThetaIn", "1.5"));
    TS_ASSERT_THROWS_NOTHING(ConvertSingleSpectrumLambdaToQ.setPropertyValue(
        "Target", "MomentumTransfer"));
    TS_ASSERT_THROWS_NOTHING(ConvertSingleSpectrumLambdaToQ.execute());
    TS_ASSERT(ConvertSingleSpectrumLambdaToQ.isExecuted());

    MatrixWorkspace_sptr input;
    TS_ASSERT_THROWS_NOTHING(
        input = AnalysisDataService::Instance().retrieve(inputWs));

    MatrixWorkspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(
        output = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(
            "outputWs"));

    // Check that the output unit is correct
    TS_ASSERT_EQUALS(output->getAxis(0)->unit()->unitID(), "MomentumTransfer");

    // Check that X has been correctly converted
    TS_ASSERT_DELTA(output->x(0)[0], 2.2429e-5, 1e-9);
    TS_ASSERT_DELTA(output->x(0)[1], 4.4859e-5, 1e-9);
    TS_ASSERT_DELTA(output->x(0)[1], 2.9597e-5, 1e-9);

    // Check that Y and E were correctly reversed
    TS_ASSERT_DELTA(output->y(0)[0], input->y(0)[2], 1e-6);
    TS_ASSERT_DELTA(output->y(0)[1], input->y(0)[1], 1e-6);
    TS_ASSERT_DELTA(output->y(0)[2], input->y(0)[0], 1e-6);
    TS_ASSERT_DELTA(output->e(0)[0], input->e(0)[2], 1e-6);
    TS_ASSERT_DELTA(output->e(0)[1], input->e(0)[1], 1e-6);
    TS_ASSERT_DELTA(output->e(0)[2], input->e(0)[0], 1e-6);

    AnalysisDataService::Instance().remove("outputWs");
  }

  /// Tests the execution of the algorithm with a multi-spectra lambda
  /// workspace
  void testExecMultiSpectraLambdaInputThrows() {
    setUpMultiSpectraLambdaWs(inputWs);

    ConvertSingleSpectrumLambdaToQ ConvertSingleSpectrumLambdaToQ;
    TS_ASSERT_THROWS_NOTHING(ConvertSingleSpectrumLambdaToQ.initialize());
    TS_ASSERT(ConvertSingleSpectrumLambdaToQ.isInitialized());
    TS_ASSERT_THROWS(ConvertSingleSpectrumLambdaToQ.setPropertyValue(
        "InputWorkspace", inputWs));
    TS_ASSERT(!ConvertSingleSpectrumLambdaToQ.isExecuted());

    MatrixWorkspace_sptr input;
    TS_ASSERT_THROWS_NOTHING(
        input = AnalysisDataService::Instance().retrieve(inputWs));

    MatrixWorkspace_sptr output;
    TS_ASSERT_THROWS(
        output = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(
            "outputWs"));

    // Check that the unit remains unchanged
    TS_ASSERT_EQUALS(input->getAxis(0)->unit()->unitID(), "Wavelength");
  }

  /// Tests the execution of the algorithm with a single-spectrum momentum
  /// workspace
  void testExecSingleSpectrumMomentumInputThrows() {
    setUpSingleSpectrumMomentumWs(inputWs);

    ConvertSingleSpectrumLambdaToQ ConvertSingleSpectrumLambdaToQ;
    TS_ASSERT_THROWS_NOTHING(ConvertSingleSpectrumLambdaToQ.initialize());
    TS_ASSERT(ConvertSingleSpectrumLambdaToQ.isInitialized());
    TS_ASSERT_THROWS(ConvertSingleSpectrumLambdaToQ.setPropertyValue(
        "InputWorkspace", inputWs));
    TS_ASSERT(!ConvertSingleSpectrumLambdaToQ.isExecuted());

    MatrixWorkspace_sptr input;
    TS_ASSERT_THROWS_NOTHING(
        input = AnalysisDataService::Instance().retrieve(inputWs));

    MatrixWorkspace_sptr output;
    TS_ASSERT_THROWS(
        output = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(
            "outputWs"));

    // Check that the unit remains unchanged
    TS_ASSERT_EQUALS(input->getAxis(0)->unit()->unitID(), "Wavelength");
  }

private:
  ConvertSingleSpectrumLambdaToQ alg;
  std::string inputWs;
  std::string outputWs;
};

class ConvertSingleSpectrumLambdaToQTestPerformance
    : public CxxTest::TestSuite {
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static ConvertSingleSpectrumLambdaToQTestPerformance *createSuite() {
    return new ConvertSingleSpectrumLambdaToQTestPerformance();
  }
  static void
  destroySuite(ConvertSingleSpectrumLambdaToQTestPerformance *suite) {
    delete suite;
  }

  void setUp() override { inputWs = "testWorkspace"; }

  void tearDown() override {
    AnalysisDataService::Instance().remove("outputWs");
  }

  void test_single_spectrum_lambda_workspace() {

    setUpSingleSpectrumLambdaWs(inputWs);

    alg.initialize();
    alg.setPropertyValue("InputWorkspace", "inputWs");
    alg.setPropertyValue("OutputWorkspace", "outputWs");
    alg.setPropertyValue("ThetaIn", "1.5");
    alg.setPropertyValue("Target", "MomentumTransfer");
    TS_ASSERT_THROWS_NOTHING(alg.execute());
    TS_ASSERT(alg.isExecuted());
  }

  void test_multi_spectra_lambda_workspace() {

    setUpMultiSpectraLambdaWs(inputWs);

    alg.initialize();
    alg.setPropertyValue("InputWorkspace", "inputWs");
    alg.setPropertyValue("OutputWorkspace", "outputWs");
    alg.setPropertyValue("ThetaIn", "1.5");
    alg.setPropertyValue("Target", "MomentumTransfer");
    TS_ASSERT_THROWS(alg.execute());
    TS_ASSERT(!alg.isExecuted());
  }

  void test_single_spectrum_momentum_workspace() {

    setUpSingleSpectrumMomentumWs(inputWs);

    alg.initialize();
    alg.setPropertyValue("InputWorkspace", "inputWs");
    alg.setPropertyValue("OutputWorkspace", "outputWs");
    alg.setPropertyValue("ThetaIn", "1.5");
    alg.setPropertyValue("Target", "MomentumTransfer");
    TS_ASSERT_THROWS(alg.execute());
    TS_ASSERT(!alg.isExecuted());
  }

private:
  ConvertSingleSpectrumLambdaToQ alg;
  std::string inputWs;
  std::string outputWs;
};
