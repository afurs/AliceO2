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

#include "CommonUtils/ConfigurableParam.h"
#include "Framework/ConfigParamSpec.h"
#include "Framework/ConfigContext.h"
#include "Framework/DeviceSpec.h"
#include "Framework/WorkflowSpec.h"
#include "Framework/Task.h"
#include "FT0Base/Constants.h"
#include "DetectorsBase/GRPGeomHelper.h"
#include "DataFormatsFT0/ChannelData.h"
#include "DataFormatsFT0/Digit.h"
#include "DataFormatsFT0/DigitFilterParam.h"
#include "DataFormatsFIT/AmpTimeDistribution.h"
#include "CommonDataFormat/BunchFilling.h"
#include <fmt/format.h>
#include <memory>
#include <filesystem>

#include "TList.h"
#include "TFile.h"

using namespace o2::framework;

namespace o2::ft0
{

class FT0AmpTimeSpectraProcessor final : public o2::framework::Task
{

 public:
  FT0AmpTimeSpectraProcessor(std::shared_ptr<o2::base::GRPGeomRequest> gr) : mGGCCDBRequest(gr) {}
  ~FT0AmpTimeSpectraProcessor() override = default;
  using AmpTimeDistributionPerADC = std::array<o2::fit::AmpTimeDistribution, Constants::sNCHANNELS>;
  std::array<AmpTimeDistributionPerADC, Constants::sNADC> mArrAmpTimeDistribution;
  typename o2::BunchFilling::Pattern mBCpattern{};
  // std::array<AmpTimeDistributionPerADC, Constants::sNADC> mArrAmpTimeDistributionTotal;
  std::shared_ptr<o2::base::GRPGeomRequest> mGGCCDBRequest;
  int mNbinsY{400};
  float mMinY{-200.};
  float mMaxY{200.};
  int mAmpThreshold{10};
  int mTimeWindow{153};
  int mBinsInStep{50};
  int mNTFtoProcess{1024};
  int mNTFprocessed{0};
  uint64_t mIteration{0};
  bool mDumpHists{false};
  std::string mDirPathHistDump{""};
  size_t mNBins{0};
  uint8_t mPMbitsToCheck{0b11111110};
  uint8_t mPMbitsGood{0b01001000};
  uint64_t mTrgBitsToCheck{0b11110000};
  uint64_t mTrgBitsGood{0b10010000};
  std::vector<typename o2::fit::AmpTimeDistribution::Content_t> mContent;
  void initHists()
  {
    mNBins = 0;
    for (int iAdc = 0; iAdc < Constants::sNADC; iAdc++) {
      for (int iCh = 0; iCh < Constants::sNCHANNELS; iCh++) {
        const std::string name = fmt::format("hAmpVsTime_adc{}_ch{}", iAdc, iCh);
        const std::string title = fmt::format("Amp Vs Time ADC{} channelID {}; Amp [ADC]; Time [TDC]", iAdc, iCh);
        mArrAmpTimeDistribution[iAdc][iCh] = o2::fit::AmpTimeDistribution(name, title, mNbinsY, mMinY, mMaxY, mBinsInStep, 4095, 0);
        mNBins += mArrAmpTimeDistribution[iAdc][iCh].mHist->GetNcells();
      }
      /*
      for(int iCh=0; iCh<Constants::sNCHANNELS; iCh++) {
        const std::string name = fmt::format("hAmpVsTime_adc{}_ch{}_total", iAdc, iCh);
        const std::string title = fmt::format("Amp Vs Time ADC{} channelID {}; Amp [ADC]; Time [TDC]", iAdc, iCh);
        mArrAmpTimeDistributionTotal[iAdc][iCh] = o2::fit::AmpTimeDistribution(name, title, mNbinsY, mMinY, mMaxY, mBinsInStep, 4095, 0);
      }
      */
    }
    mContent.resize(mNBins);
  }
  void fillContent()
  {
    size_t startPos = 0;
    for (int iAdc = 0; iAdc < Constants::sNADC; iAdc++) {
      for (int iCh = 0; iCh < Constants::sNCHANNELS; iCh++) {
        startPos = mArrAmpTimeDistribution[iAdc][iCh].serialize(mContent, startPos);
      }
    }
  }
  void resetHists()
  {
    for (int iAdc = 0; iAdc < Constants::sNADC; iAdc++) {
      for (int iCh = 0; iCh < Constants::sNCHANNELS; iCh++) {
        mArrAmpTimeDistribution[iAdc][iCh].mHist->Reset();
      }
    }
  }
  void dumpHists(const std::string& filepath) const
  {
    TFile tfile(filepath.c_str(), "RECREATE");
    auto tlist = std::make_unique<TList>();
    tlist->SetOwner(false);
    tlist->SetName("output");
    for (int iAdc = 0; iAdc < Constants::sNADC; iAdc++) {
      for (int iCh = 0; iCh < Constants::sNCHANNELS; iCh++) {
        tlist->Add(mArrAmpTimeDistribution[iAdc][iCh].mHist.get());
      }
    }
    tfile.WriteObject(tlist.get(), tlist->GetName(), "SingleKey");
    tfile.Close();
  }
  void dumpHistsForIteration() const
  {
    const std::string filename = fmt::format("AmpTimeDistributionFT0_chunk{:05}.root", mIteration);
    const auto fullFilepath = std::filesystem::path{mDirPathHistDump} / std::filesystem::path{filename};
    dumpHists(fullFilepath.string());
  }
  void init(o2::framework::InitContext& ic) final
  {
    o2::base::GRPGeomHelper::instance().setRequest(mGGCCDBRequest);
    mNbinsY = ic.options().get<int>("number-bins-y");
    mMinY = ic.options().get<float>("low-edge-y");
    mMaxY = ic.options().get<float>("upper-edge-y");
    mBinsInStep = ic.options().get<int>("step-bins-x-axis");
    mNTFtoProcess = ic.options().get<int>("ntfs-as-chunk");
    mDirPathHistDump = ic.options().get<std::string>("dirpath-dump-hists");
    mDumpHists = mDirPathHistDump.size() > 0;
    LOG(info) << "Histogram parameters: " << mNbinsY << " " << mMinY << " " << mMaxY;
    const auto& param = o2::ft0::DigitFilterParam::Instance();
    param.printKeyValues();
    mAmpThreshold = param.mAmpThreshold;
    mTimeWindow = param.mTimeWindow;
    mPMbitsGood = param.mPMbitsGood;
    mPMbitsToCheck = param.mPMbitsToCheck;
    mTrgBitsGood = param.mTrgBitsGood;
    mTrgBitsToCheck = param.mTrgBitsToCheck;
    initHists();
  }
  void run(o2::framework::ProcessingContext& pc) final
  {
    o2::base::GRPGeomHelper::instance().checkUpdates(pc);
    mNTFprocessed++;
    auto digits = pc.inputs().get<gsl::span<o2::ft0::Digit>>("digits");
    auto channels = pc.inputs().get<gsl::span<o2::ft0::ChannelData>>("channels");
    for (const auto& digit : digits) {
      const uint64_t trgWordExt = digit.mTriggers.getExtendedTrgWordFT0();
      const auto bc = digit.getIntRecord().bc;
      if (!mBCpattern.test(bc) || (trgWordExt & mTrgBitsToCheck) != mTrgBitsGood) {
        continue;
      }
      const auto& chan = digit.getBunchChannelData(channels);
      for (const auto& channel : chan) {
        if (channel.ChId > 207)
          continue;
        const int adc = (channel.ChainQTC >> o2::ft0::ChannelData::kNumberADC) & 1;
        auto& hist = mArrAmpTimeDistribution[adc][channel.ChId].mHist;
        hist->Fill(channel.QTCAmpl, channel.CFDTime);
        //        auto &histTotal = mArrAmpTimeDistributionTotal[adc][channel.ChId].mHist;
        //        histTotal->Fill(channel.QTCAmpl, channel.CFDTime);
      }
    }
    if (mNTFprocessed > mNTFtoProcess) {
      if (mDumpHists) {
        dumpHistsForIteration();
      }
      //      fillContent();
      resetHists();
      //      pc.outputs().snapshot(o2::framework::Output{o2::header::gDataOriginFT0, "AMP_TIME_SPECTRA", 0}, mContent);
      mNTFprocessed = 0;
      mIteration++;
    }
  }
  void finaliseCCDB(ConcreteDataMatcher& matcher, void* obj)
  {
    if (o2::base::GRPGeomHelper::instance().finaliseCCDB(matcher, obj)) {
      mBCpattern = o2::base::GRPGeomHelper::instance().getGRPLHCIF()->getBunchFilling().getBCPattern();
      LOG(info) << "BUNCH FILLING: " << mBCpattern.to_string();
      return;
    }
  }
  void endOfStream(EndOfStreamContext&)
  {
    if (mDumpHists) {
      dumpHistsForIteration();
    }
  }
};

} // namespace o2::ft0

void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
{
  std::vector<ConfigParamSpec> options;
  options.push_back(ConfigParamSpec{"dispatcher-mode", VariantType::Bool, false, {"Dispatcher mode (FT0/SUB_DIGITSCH and FT0/SUB_DIGITSBC DPL channels should be applied as dispatcher output)."}});
  options.push_back(ConfigParamSpec{"configKeyValues", VariantType::String, "", {"Semicolon separated key=value strings"}});
  std::swap(workflowOptions, options);
}

#include "Framework/runDataProcessing.h"

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  Inputs inputs{};
  if (cfgc.options().get<bool>("dispatcher-mode")) {
    inputs.push_back(InputSpec{{"channels"}, "FT0", "SUB_DIGITSCH"});
    inputs.push_back(InputSpec{{"digits"}, "FT0", "SUB_DIGITSBC"});
  } else {
    inputs.push_back(InputSpec{{"channels"}, "FT0", "DIGITSCH"});
    inputs.push_back(InputSpec{{"digits"}, "FT0", "DIGITSBC"});
  }
  o2::conf::ConfigurableParam::updateFromString(cfgc.options().get<std::string>("configKeyValues"));
  auto ggRequest = std::make_shared<o2::base::GRPGeomRequest>(false,                          // orbitResetTime
                                                              false,                          // GRPECS=true
                                                              true,                           // GRPLHCIF
                                                              false,                          // GRPMagField
                                                              false,                          // askMatLUT
                                                              o2::base::GRPGeomRequest::None, // geometry
                                                              inputs,
                                                              true);
  DataProcessorSpec dataProcessorSpec{
    "FT0AmpTimeSpectraProcessor",
    inputs,
    Outputs{
      {{"ampTimeSpectra"}, "FT0", "AMP_TIME_SPECTRA"}},
    AlgorithmSpec{adaptFromTask<o2::ft0::FT0AmpTimeSpectraProcessor>(ggRequest)},
    Options{
      {"dirpath-dump-hists", VariantType::String, "", {"Provide directory where to dump amp-time histograms. If empty - no dumping."}},
      {"ntfs-as-chunk", VariantType::Int, 1024, {"Number of timeframes to process as single chunk. After processing n TFs, result will be sent to next device."}},
      {"number-bins-y", VariantType::Int, 200, {"Number of bins along Y-axis"}},
      {"low-edge-y", VariantType::Float, -100.0f, {"Lower edge of first bin along Y-axis"}},
      {"upper-edge-y", VariantType::Float, 100.0f, {"Upper edge of last bin along Y-axis"}},
      {"step-bins-x-axis", VariantType::Int, 50, {"Step for variable bin axis production, i.e. number of bins for step i with 2^i bin width"}}}};

  WorkflowSpec workflow;
  workflow.emplace_back(dataProcessorSpec);

  return workflow;
}
