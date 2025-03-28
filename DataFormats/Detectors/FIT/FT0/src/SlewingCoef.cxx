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

#include <string>
#include <map>
#include <ROOT/RCsvDS.hxx>
#include <cassert>

#include "DataFormatsFT0/SlewingCoef.h"

using namespace o2::ft0;

SlewingCoef::SlewingPlots_t SlewingCoef::makeSlewingPlots() const
{
  typename o2::ft0::SlewingCoef::SlewingPlots_t plots{};
  for (int iAdc = 0; iAdc < sNAdc; iAdc++) {
    const auto& slewingCoefs = mSlewingCoefs[iAdc];
    auto& plotsAdc = plots[iAdc];
    for (int iCh = 0; iCh < sNCHANNELS; iCh++) {
      const auto& points_x = slewingCoefs[iCh].first;
      const auto& points_y = slewingCoefs[iCh].second;
      assert(points_x.size() == points_y.size());
      const int nPoints = points_x.size();
      auto& plot = plotsAdc[iCh];
      plot = TGraph(nPoints, points_x.data(), points_y.data());
    }
  }
  return plots;
}

void SlewingCoef::fromCSV(const std::string& filepathSlewing, const std::string& filepathOffset)
{
  std::map<std::pair<int, int>, double> mapOffsets{}; // std::pair<int, int>{channelID, adc}
  if (filepathOffset.size() > 0) {
    auto dfOffset = ROOT::RDF::FromCSV(filepathOffset.c_str(), true, ';', -1LL);
    dfOffset.Foreach([&mapOffsets](const Long64_t& chID, const Long64_t& adc, const double peak) {
      mapOffsets.insert({{chID, adc}, peak});
    },
                     {"channelID", "ADC", "peak"});
  }
  auto dfSlewing = ROOT::RDF::FromCSV(filepathSlewing.c_str(), true, ';', -1LL);
  dfSlewing.Foreach([this, &mapOffsets](const Long64_t& chID, const Long64_t& adc, const double& x_min,
                                        const double& y_min, const double& x_max, const double& y_max) {
    const auto& it = mapOffsets.find({chID, adc});
    const double offset = 0. ? it == mapOffsets.end() : it->second;
    auto& pointsX = mSlewingCoefs[adc][chID].first;
    auto& pointsY = mSlewingCoefs[adc][chID].second;
    pointsX.emplace_back(x_min);
    pointsX.emplace_back(x_max);
    pointsY.emplace_back(y_min - offset);
    pointsY.emplace_back(y_max - offset);
  },
                    {"channelID", "ADC", "x_min", "y_min", "x_max", "y_max"});
}