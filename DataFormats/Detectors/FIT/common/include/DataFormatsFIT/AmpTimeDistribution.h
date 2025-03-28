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

#ifndef O2_FITAMPTIMEDISTRIBUTION_H
#define O2_FITAMPTIMEDISTRIBUTION_H

#include "TH2F.h"

#include <string>
#include <memory>
#include <utility>
#include <vector>

#include <Vc/Vc>
#include <span>

namespace o2::fit
{

struct AmpTimeDistribution {
  AmpTimeDistribution() = default;
  AmpTimeDistribution(const std::string& name, const std::string& title, int nBins, double minRange, double maxRange, int binsInStep = 50, int binMax = 4095, int axis = 0);
  AmpTimeDistribution(const AmpTimeDistribution&);
  AmpTimeDistribution(AmpTimeDistribution&&) noexcept;
  AmpTimeDistribution& operator=(const AmpTimeDistribution&);
  AmpTimeDistribution& operator=(AmpTimeDistribution&&) noexcept;
  typedef float Content_t; // to template?
  typedef TH2F Hist2F_t;
  typedef Vc::float_v VcContent_t;
  std::unique_ptr<Hist2F_t> mHist = nullptr;
  // TODO: not finished yet, for perspective
  struct Header { // for serialization/deserialization
    static constexpr std::size_t sNFirstFields = 5;
    static constexpr std::size_t sSizeFirstFields = sizeof(Content_t) * sNFirstFields;
    // First five double fields
    Content_t mNBinConstant;
    Content_t mMinBinConstant;
    Content_t mMaxBinConstant;
    Content_t mNEdgesVariable; // N elements to specialize edges
    Content_t mAxisConstant;   // Which axis has constant bin width, 0 -> X, 1 -> Y
    //
    std::vector<Content_t> mVecBinEdge;

    std::size_t serialize(std::vector<Content_t>& vecDst, std::size_t startPos) const
    {
      std::memcpy(&vecDst[startPos], this, sSizeFirstFields);
      startPos += sNFirstFields;
      std::memcpy(&vecDst[startPos], mVecBinEdge.data(), sizeof(Content_t) * mVecBinEdge.size());
      startPos += mVecBinEdge.size();
      return startPos;
    }

    std::size_t deserialize(std::vector<Content_t>& vecSrc, std::size_t startPos)
    {
      std::memcpy(this, &vecSrc[startPos], sSizeFirstFields);
      startPos += sNFirstFields;
      const std::size_t nElementsVec = static_cast<std::size_t>(mNEdgesVariable);
      std::memcpy(&vecSrc[startPos], mVecBinEdge.data(), sizeof(Content_t) * nElementsVec);
      startPos += nElementsVec;
      return startPos;
    }
    std::size_t getTotalN() const
    {
      return mVecBinEdge.size() + 5;
    }
  };

  void initHists(const std::string& name, const std::string& title, int nBins, double minRange, double maxRange, int binsInStep = 50, int binMax = 4095, int axis = 0);
  static std::vector<double> makeVaribleBins(const std::vector<std::pair<int, int>>& vecParams, int binMax = 4095);
  static std::vector<double> makeVaribleBins(int binsInStep = 50, int binMax = 4095);
  std::size_t serialize(std::vector<Content_t>& vecDst, size_t startPos);
  std::size_t deserialize(std::vector<Content_t>& vecDst, size_t startPos);
  std::size_t addContent(const std::span<const Content_t>& vecSrc, size_t startPos);
};

template <std::size_t NCHANNELS, std::size_t NADC>
using AmpTimeDistributionDetector = std::array<std::array<AmpTimeDistribution, NCHANNELS>, NADC>;

} // namespace o2::fit
#endif