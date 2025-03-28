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

#ifndef O2_FT0SLEWINGSLOTCONTAINER_H
#define O2_FT0SLEWINGSLOTCONTAINER_H

#include <bitset>
#include <array>

#include "DataFormatsFT0/SlewingCoef.h"
#include "DataFormatsFIT/AmpTimeDistribution.h"

#include "DetectorsCalibration/TimeSlotCalibration.h"
#include "DetectorsCalibration/TimeSlot.h"
#include "Framework/InitContext.h"

#include "TList.h"

#include "Rtypes.h"
namespace o2::ft0
{

class FT0SlewingSlotContainer final
{
  static constexpr int sNCHANNELS = o2::ft0::Geometry::Nchannels;
  using TimeSlot = o2::calibration::TimeSlot<FT0TimeOffsetSlotContainer>;
  using TimeSlotCalibration = o2::calibration::TimeSlotCalibration<FT0TimeOffsetSlotContainer>;

 public:
  FT0SlewingSlotContainer(std::size_t minEntries); // constructor is needed due to current version of FITCalibration library, should be removed
  FT0SlewingSlotContainer(FT0SlewingSlotContainer const&) = default;
  FT0SlewingSlotContainer(FT0SlewingSlotContainer&&) = default;
  FT0SlewingSlotContainer& operator=(FT0SlewingSlotContainer&) = default;
  FT0SlewingSlotContainer& operator=(FT0SlewingSlotContainer&&) = default;
  using AmpTimeDistributionPerADC = std::array<o2::fit::AmpTimeDistribution, Constants::sNCHANNELS>;
  using AmpTimeDistributionTotal = std::array<AmpTimeDistributionPerADC, Constants::sNADC> typedef std::remove_cvref_t<std::remove_pointer_t<o2::fit::AmpTimeDistribution::Content_t>> Content_t;
  bool hasEnoughEntries() const;
  void fill(const gsl::span<const Content_t>& data);
  void initCtx(o2::framework::InitContext&);
  void merge(FT0SlewingSlotContainer* prev);
  void print() const;
  o2::ft0::SlewingCoef generateCalibrationObject(long tsStartMS, long tsEndMS) const;

  auto isFirstTF() const { return mIsFirstTF; }

 private:
  bool mIsHistsReady{false};
  // Options for hist parameters
  // TODO: obtain from data stream, not from options
  int mNbinsY{400};
  float mMinY{-200.};
  float mMaxY{200.};
  int mBinsInStep{50};
  std::array<AmpTimeDistributionPerADC, Constants::sNADC> mArrAmpTimeDistribution;

  // Slot number
  uint8_t mCurrentSlot = 0;
  // Status of channels, pending channels = !(good | bad)
  std::bitset<sNCHANNELS> mBitsetBadChIDs;
  std::bitset<sNCHANNELS> mBitsetGoodChIDs;
  // For hist init, for making hist ranges dynamic
  bool mIsFirstTF{true};
  // For slot finalizing
  bool mIsReady{false};
  // Hist init
  void intHists();
  std::size_t mNBins{0};
  // Once it is upper than max entry threshold it stops increasing
  std::array<std::size_t, sNCHANNELS> mArrEntries{};
  // Total number of events
  uint64_t mTotalNevents{0};
  // Contains all information about time spectra
  ClassDefNV(FT0SlewingSlotContainer, 1);
};
} // namespace o2::ft0

#endif // O2_FT0SLEWINGSLOTCONTAINER_H
