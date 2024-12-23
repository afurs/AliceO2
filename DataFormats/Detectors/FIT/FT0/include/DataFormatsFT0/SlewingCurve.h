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

#ifndef O2_FT0SLEWINGCURVE_H
#define O2_FT0SLEWINGCURVE_H

#include "TH2F.h"

#include <string>
#include <memory>
#include <utility>
#include <vector>

namespace o2::ft0
{

struct SlewingCurve {
  typedef TH2F Hist2D_t;
  std::unique_ptr<Hist2D_t> mHist=nullptr;
  SlewingCurve() = default;
  SlewingCurve(const std::string &name, const std::string &title, int nBins, double minRange, double maxRange, int binsInStep=50, int binMax = 4095, int axis=0);

  void initHists(const std::string &name, const std::string &title, int nBins, double minRange, double maxRange, int binsInStep=50, int binMax = 4095, int axis=0);
  static std::vector<double> makeVaribleBins(const std::vector<std::pair<int, int> > &vecParams, int binMax=4095);
  static std::vector<double> makeVaribleBins(int binsInStep=50, int binMax = 4095);
  size_t fillContent(std::vector<Double_t> &vecDst, size_t startPos);
};
} // namespace o2::ft0
#endif