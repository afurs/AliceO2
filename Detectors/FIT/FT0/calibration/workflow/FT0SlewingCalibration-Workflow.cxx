// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Framework/DataProcessorSpec.h"

#include "CommonUtils/ConfigurableParam.h"
#include "DataFormatsFT0/SlewingCoef.h"
#include "FITCalibration/FITCalibrationDevice.h"
#include "FT0Calibration/FT0SlewingSlotContainer.h"

using namespace o2::framework;

void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
{
  // probably some option will be added
  std::vector<o2::framework::ConfigParamSpec> options;
  options.push_back(ConfigParamSpec{"configKeyValues", VariantType::String, "", {"Semicolon separated key=value strings"}});

  std::swap(workflowOptions, options);
}

#include "Framework/runDataProcessing.h"

WorkflowSpec defineDataProcessing(ConfigContext const& config)
{
  using CalibrationDeviceType = o2::fit::FITCalibrationDevice<float,
                                                              o2::ft0::FT0SlewingSlotContainer, o2::ft0::SlewingCoef>;
  std::vector<o2::framework::InputSpec> inputs;
  std::vector<o2::framework::OutputSpec> outputs;
  const o2::header::DataDescription inputDataDescriptor{"AMP_TIME_SPECTRA"};
  const o2::header::DataDescription outputDataDescriptor{"FT0_SLEW_CALIB"};
  CalibrationDeviceType::prepareVecInputSpec(inputs, o2::header::gDataOriginFT0, inputDataDescriptor);
  CalibrationDeviceType::prepareVecOutputSpec(outputs, outputDataDescriptor);

  o2::conf::ConfigurableParam::updateFromString(config.options().get<std::string>("configKeyValues"));
  auto ccdbRequest = std::make_shared<o2::base::GRPGeomRequest>(true,                           // orbitResetTime
                                                                true,                           // GRPECS=true
                                                                false,                          // GRPLHCIF
                                                                false,                          // GRPMagField
                                                                false,                          // askMatLUT
                                                                o2::base::GRPGeomRequest::None, // geometry
                                                                inputs);
  o2::framework::DataProcessorSpec dataProcessorSpec{
    "ft0-slew-calib",
    inputs,
    outputs,
    o2::framework::AlgorithmSpec{o2::framework::adaptFromTask<CalibrationDeviceType>(ccdbRequest, outputDataDescriptor)},
    o2::framework::Options{
      {"tf-per-slot", o2::framework::VariantType::UInt32, 56000u, {""}},
      {"max-delay", o2::framework::VariantType::UInt32, 3u, {""}},
      {"extra-info-per-slot", o2::framework::VariantType::String, "", {"Extra info for time slot(usually for debugging)"}},
      {"number-bins-y", VariantType::Int, 200, {"Number of bins along Y-axis"}},
      {"low-edge-y", VariantType::Float, -100.0f, {"Lower edge of first bin along Y-axis"}},
      {"upper-edge-y", VariantType::Float, 100.0f, {"Upper edge of last bin along Y-axis"}},
      {"step-bins-x-axis", VariantType::Int, 50, {"Step for variable bin axis production, i.e. number of bins for step i with 2^i bin width"}},
      {"dump-hists", VariantType::String, "", {"Dump hists into file"}}}};

  WorkflowSpec workflow;
  workflow.emplace_back(dataProcessorSpec);
  return workflow;
}
