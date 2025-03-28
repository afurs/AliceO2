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

#include "FT0Calibration/FT0SlewingSlotContainer.h"
#include "DataFormatsFT0/CalibParam.h"
#include <Framework/Logger.h>
#include <Framework/ConfigParamRegistry.h>

#include "TH1.h"
#include "TFile.h"
#include "TFitResult.h"
using namespace o2::ft0;

FT0SlewingSlotContainer::FT0SlewingSlotContainer(std::size_t minEntries) {}

bool FT0SlewingSlotContainer::hasEnoughEntries() const
{
  if (mTotalNevents == 0) {
    // Dummy slot, should be ignored for protection
    LOG(warning) << "RESULT: Empty slot, ignoring";
    return false;
  } else if (mIsReady) {
    // ready : bad+good == NChannels (i.e. no pending channel)
    LOG(info) << "RESULT: ready";
    print();
    return true;
  } else if (mCurrentSlot >= CalibParam::Instance().mNExtraSlots) {
    LOG(info) << "RESULT: Extra slots(" << CalibParam::Instance().mNExtraSlots << ") are used";
    print();
    return true;
  } else if (mCurrentSlot < CalibParam::Instance().mNExtraSlots) {
    for (int iCh = 0; iCh < sNCHANNELS; iCh++) {
      const auto nEntries = mArrEntries[iCh];
      if (nEntries >= CalibParam::Instance().mMinEntriesThreshold && nEntries < CalibParam::Instance().mMaxEntriesThreshold) {
        // Check if there are any pending channel in first slot
        LOG(info) << "RESULT: pending channels";
        print();
        return false;
      }
    }
    // If sum of bad+good == NChannels (i.e. no pending channel in first slot)
    LOG(info) << "RESULT: NO pending channels";
    print();
    return true;
  } else {
    // Probably will be never happen, all other conditions are already checked
    LOG(info) << "RESULT: should be never happen";
    print();
    return true;
  }
  return true;
}

void FT0SlewingSlotContainer::fill(const gsl::span<const float>& data)
{
  std::size_t startPos = 0;
  for (int iAdc = 0; iAdc < Constants::sNADC; iAdc++) {
    for (int iCh = 0; iCh < Constants::sNCHANNELS; iCh++) {
      startPos = mArrAmpTimeDistribution[iCh][iAdc].addContent(data, startPos);
      const auto nEntries = mArrAmpTimeDistribution[iAdc][iCh].mHist.GetEntries();
      mArrEntries[iAdc][iCh] = nEntries;
      mTotalNevents += nEntries;
      if (nEntries >= CalibParam::Instance().mMaxEntriesThreshold) {
        mBitsetGoodChIDs.set(iCh);
      }
      const auto totalNCheckedChIDs = mBitsetGoodChIDs.count() + mBitsetBadChIDs.count();
      if (totalNCheckedChIDs == sNCHANNELS) {
        mIsReady = true;
      }
    }
  }
}

void FT0SlewingSlotContainer::merge(FT0TimeOffsetSlotContainer* prev)
{
  LOG(info) << "MERGING";
  /*
    if (mIsFirstTF && prev->isFirstTF()) {
      // nothing to be done
      return;
    } else if (mIsFirstTF && !prev->isFirstTF()) {
  //    mHistogram.init(prev->getHistogram().getNBinsX(), prev->getHistogram().getXMin(), prev->getHistogram().getXMax(), prev->getHistogram().getNBinsY(), prev->getHistogram().getYMin(), prev->getHistogram().getYMax());
  //    mIsFirstTF = false;
    }
  */
  *this = std::move(*prev);
  /*
    if (mCurrentSlot == 0) {
      // This part should at the stage `hasEnoughData()` but it is const method
      for (int iCh = 0; iCh < sNCHANNELS; iCh++) {
        if (mArrEntries[iCh] < CalibParam::Instance().mMinEntriesThreshold) {
          // If in first slot channel entries below range => set status bad
          mBitsetBadChIDs.set(iCh);
        }
      }
    }
    this->print();
  */
  if (mCurrentSlot == 0) {
    // This part should at the stage `hasEnoughData()` but it is const method
    for (int iCh = 0; iCh < sNCHANNELS; iCh++) {
      if (mArrEntries[iCh] < CalibParam::Instance().mMinEntriesThreshold) {
        // If in first slot channel entries below range => set status bad
        mBitsetBadChIDs.set(iCh);
      }
    }
  }
  mCurrentSlot++;
}

void initHists()
{
  mNBins = 0;
  for (int iAdc = 0; iAdc < Constants::sNADC; iAdc++) {
    for (int iCh = 0; iCh < Constants::sNCHANNELS; iCh++) {
      const std::string name = fmt::format("hAmpVsTime_ch{}_adc{}", iCh, iAdc);
      const std::string title = fmt::format("Amp Vs Time channelID {} ADC{}; Amp [ADC]; Time [TDC]", iCh, iAdc);
      mArrAmpTimeDistribution[iAdc][iCh] = o2::fit::AmpTimeDistribution(name, title, mNbinsY, mMinY, mMaxY, mBinsInStepX, 4095, 0);
      mNBins += mArrAmpTimeDistribution[iAdc][iCh].mHist->GetNcells();
    }
  }
  mIsHistsReady = true;
}

void FT0SlewingSlotContainer::initCtx(o2::framework::InitContext& ctx)
{
  mNbinsY = ctx.options().get<int>("number-bins-y");
  mMinY = ctx.options().get<float>("low-edge-y");
  mMaxY = ctx.options().get<float>("upper-edge-y");
  mBinsInStep = ctx.options().get<int>("step-bins-x-axis");
  initHists();
  // LOG(info) << "Histogram parameters: " << mNbinsY << " " << mMinY << " " << mMaxY;
}
SpectraInfoObject FT0SlewingSlotContainer::getSpectraInfoObject(std::unique_ptr<TH1F>& hist) const
{
  uint32_t statusBits{};
  double minFitRange{0};
  double maxFitRange{0};
  if (channelID < sNCHANNELS) {
    if (CalibParam::Instance().mRebinFactorPerChID[channelID] > 0) {
      hist->Rebin(CalibParam::Instance().mRebinFactorPerChID[channelID]);
    }
  }
  const float meanHist = hist->GetMean();
  const float rmsHist = hist->GetRMS();
  const float stat = hist->Integral();
  if (CalibParam::Instance().mUseDynamicRange) {
    minFitRange = meanHist - CalibParam::Instance().mRangeInRMS * rmsHist;
    maxFitRange = meanHist + CalibParam::Instance().mRangeInRMS * rmsHist;
  } else {
    minFitRange = CalibParam::Instance().mMinFitRange;
    maxFitRange = CalibParam::Instance().mMaxFitRange;
  }
  float constantGaus{};
  float meanGaus{};
  float sigmaGaus{};
  float fitChi2{};
  if (stat > 0) {
    TFitResultPtr resultFit = hist->Fit("gaus", "0SQ", "", minFitRange, maxFitRange);
    if (((int)resultFit) == 0) {
      constantGaus = resultFit->Parameters()[0];
      meanGaus = resultFit->Parameters()[1];
      sigmaGaus = resultFit->Parameters()[2];
      fitChi2 = resultFit->Chi2();
      statusBits |= (1 << 0);
    }
    if (((int)resultFit) != 0 || std::abs(meanGaus - meanHist) > CalibParam::Instance().mMaxDiffMean || rmsHist < CalibParam::Instance().mMinRMS || sigmaGaus > CalibParam::Instance().mMaxSigma) {
      statusBits |= (2 << 0);
      LOG(debug) << "Bad gaus fit: meanGaus " << meanGaus << " sigmaGaus " << sigmaGaus << " meanHist " << meanHist << " rmsHist " << rmsHist << "resultFit " << ((int)resultFit);
    }
  }
  if (listHists != nullptr) {
    auto histPtr = hist.release();
    const std::string histName = "histCh" + std::to_string(channelID);
    histPtr->SetName(histName.c_str());
    listHists->Add(histPtr);
  }
  return SpectraInfoObject{meanGaus, sigmaGaus, constantGaus, fitChi2, meanHist, rmsHist, stat, statusBits};
}

SlewingCoefs FT0SlewingSlotContainer::generateCalibrationObject(long tsStartMS, long tsEndMS) const
{
  std::unique_ptr<TList> listHists = std::make_unique<TList>();
  listHists->SetName("output");
  listHists->SetOwner(false);
  bool storeHists{mDumpToFile.size() > 0};

  SlewingCoefs calibrationObject;
  return calibrationObject;
}

void FT0SlewingSlotContainer::print() const
{
}
