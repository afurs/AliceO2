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

// \file HelperTypes.h
/// \brief Helper with metafunctions for type definitions
/// \author Artur Furs afurs@cern.ch

#ifndef O2_FIT_HELPER_TYPES_H_
#define O2_FIT_HELPER_TYPES_H_

#include <boost/mpl/map.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/placeholders.hpp>
#include <Vc/Vc>
#include "TH1.h"
namespace o2
{
namespace fit
{
namespace helper
{

template <typename T>
struct VcType {
  typedef TH2F Hist2F_t;
  typedef boost::mpl::map<
    boost::mpl::pair<float, Vc::float_v>,
    boost::mpl::pair<double, Vc::double_v>,
    boost::mpl::pair<int, Vc::int_v>,
    boost::mpl::pair<unsigned int, Vc::uint_v>,
    boost::mpl::pair<long, Vc::long_v>,
    boost::mpl::pair<unsigned long, Vc::ulong_v>,
    boost::mpl::pair<short, Vc::short_v>,
    boost::mpl::pair<unsigned short, Vc::ushort_v>,
    boost::mpl::pair<char, Vc::char_v>,
    boost::mpl::pair<unsigned char, Vc::uchar_v>,
    boost::mpl::pair<long long, Vc::longlong_v>,
    boost::mpl::pair<unsigned long long, Vc::ulonglong_v>>
    MapTypeToVc;
  typedef typename mpl::at<MapType, T>::type type;
};

template <typename T>
struct HistType {
  typedef TH2F Hist2F_t;
  typedef boost::mpl::map<
    boost::mpl::pair<float, Vc::float_v>,
    boost::mpl::pair<double, Vc::double_v>,
    boost::mpl::pair<int, Vc::int_v>,
    boost::mpl::pair<unsigned int, Vc::uint_v>,
    boost::mpl::pair<long, Vc::long_v>,
    boost::mpl::pair<unsigned long, Vc::ulong_v>,
    boost::mpl::pair<short, Vc::short_v>,
    boost::mpl::pair<unsigned short, Vc::ushort_v>,
    boost::mpl::pair<char, Vc::char_v>,
    boost::mpl::pair<unsigned char, Vc::uchar_v>,
    boost::mpl::pair<long long, Vc::longlong_v>,
    boost::mpl::pair<unsigned long long, Vc::ulonglong_v>>
    MapType;
  typedef typename mpl::at<MapType, T>::type type;
};

} // namespace helper
} // namespace fit
} // namespace o2

#endif
