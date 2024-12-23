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

#include "DataFormatsFT0/SlewingCurve.h"
#include <cstring>
using namespace o2::ft0;

SlewingCurve::SlewingCurve(const std::string &name, const std::string &title, int nBins, double minRange, double maxRange, int binsInStep, int binMax, int axis) {
  initHists(name, title, nBins, minRange, maxRange, binsInStep, binMax, axis);
}

void SlewingCurve::initHists(const std::string &name, const std::string &title, int nBins, double minRange, double maxRange, int binsInStep, int binMax, int axis) {
  const auto &varBins = makeVaribleBins(binsInStep, binMax);
  const int varNbins = varBins.size()-1;
  if(axis==0) {
    mHist = std::make_unique<Hist2D_t>(name.c_str(), title.c_str(), varNbins, varBins.data(), nBins, minRange, maxRange);
  }
  else if(axis==1) {
    mHist = std::make_unique<Hist2D_t>(name.c_str(), title.c_str(), nBins, minRange, maxRange, varNbins, varBins.data());
  }
}

std::vector<double> SlewingCurve::makeVaribleBins(const std::vector<std::pair<int, int> > &vecParams, int binMax) {
  std::vector<double> vecLowEdgeBins{};
  int startBin{0};
  auto makeBinRange = [&vec = vecLowEdgeBins, binMax](int startBin, int binWidth, int nBins) {
    const int endBin = startBin + binWidth * nBins;
    for(int iBin=startBin;iBin<endBin;iBin+=binWidth) {
      vec.emplace_back(static_cast<double>(iBin));
      if(iBin>binMax) {
        break;
      }
    }
    return endBin;
  };
  for(const auto &entry: vecParams) {
    startBin = makeBinRange(startBin , entry.first, entry.second);
  }
  return vecLowEdgeBins;
}

std::vector<double> SlewingCurve::makeVaribleBins(int binsInStep, int binMax) {
  auto generateParams = [](int binsInStep, int binMax) {
    std::vector<std::pair<int, int> > vecParams{};
    int endBin{0};
    int binWidth=1;
    while(endBin<binMax) {
      vecParams.push_back({binWidth, binsInStep});
      endBin+=(binWidth*binsInStep);
      binWidth*=2;
    }
    return vecParams;
  };
  return makeVaribleBins(generateParams(binsInStep, binMax), binMax);
}

size_t SlewingCurve::fillContent(std::vector<Double_t> &vecDst, size_t startPos) {
  const size_t cnt = sizeof(Double_t) * mHist->GetNcells();
  std::memcpy(&vecDst[startPos], mHist->fArray, cnt);
  return startPos + mHist->GetNcells();
}