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

// \file MCP.h
/// \brief Class for describing MCP entity
/// \author Artur.Furs@cern.ch

#ifndef ALICEO2_FT0_MCP_H_
#define ALICEO2_FT0_MCP_H_
#include <string>
#include <map>
#include <set>
#include <bitset>
#include <iostream>
#include <tuple>

#include "DataFormatsFT0/ChannelData.h"

namespace o2::ft0
{
struct MCP {
  /*
  ////////
  Usage example
  ////////
  std::map<unsigned int, o2::ft0::MCP> mapMCPs{};
  ...
  Fill map in ChannelData loop:

  auto itPair = mapMCPs.try_emplace(mcp::getMCPID(channelData.ChId),channelData.ChId);
  itPair.first->second.addChannelData(channelData);

  To get mcp mask for given channel ID(chID):

  const auto &it = mapMCPs.find(mcp::getMCPID(chID));
  if(it != mapMCPs.end()) {
    const auto &info = it->second.getInfo(channelData);
    const auto &mcpMask = std::get<1>(info); // EChannelMask
  }
  */
  MCP(int chID) : mMCPID(getMCPID(chID)),
                  mPatternType(getPatternType(mMCPID)) {}
  MCP(const MCP&) = default;
  MCP& operator=(const MCP&) = default;
  MCP(MCP&&)
  noexcept = default;
  MCP& operator=(MCP&&) noexcept = default;

  static constexpr unsigned int sNchPerMCP = 4;
  static constexpr unsigned int sNMCPs = 52;
  static constexpr unsigned int sAllBits = 0b1111;
  // masking channels by neighbor status
  enum class EChannelMask : int {
    kNone, // no neighbours
    kLeft,
    kRight,
    kDiag,
    kLeftRight,
    kDiagLeft, // diagonal and left neighbour activated
    kDiagRight,
    kAll, // all neighbours activeted
    kUnknown
  };

  enum class EPatternType : int {
    kPattern1,
    kPattern2,
    kPattern3,
    kNone
  };

  static const inline std::map<unsigned int, std::string> sMapMaskMCP = {{static_cast<unsigned int>(EChannelMask::kNone), "None"},
                                                                         {static_cast<unsigned int>(EChannelMask::kLeft), "Left"},
                                                                         {static_cast<unsigned int>(EChannelMask::kRight), "Right"},
                                                                         {static_cast<unsigned int>(EChannelMask::kDiag), "Diag"},
                                                                         {static_cast<unsigned int>(EChannelMask::kLeftRight), "LeftRight"},
                                                                         {static_cast<unsigned int>(EChannelMask::kDiagLeft), "DiagLeft"},
                                                                         {static_cast<unsigned int>(EChannelMask::kDiagRight), "DiagRight"},
                                                                         {static_cast<unsigned int>(EChannelMask::kAll), "All"}};
  static const inline std::set<int> sSetLocalMCPs_pattern1 = {1, 3, 5, 7, 9, 10, 13, 14, 17, 18, 21, 22, 26, 29, 32, 35, 38, 39, 42, 43, 46, 47, 50, 51};
  static const inline std::set<int> sSetLocalMCPs_pattern2 = {11, 15, 19, 23, 24, 25, 27, 28, 30, 31, 33, 34, 36, 37, 40, 41, 44, 45, 48, 49};
  static const inline std::set<int> sSetLocalMCPs_pattern3 = {0, 2, 4, 6, 8, 12, 16, 20}; // an only channels where diag: 0->2 and 1->3, instead of 0->
  //
  static EPatternType getPatternType(int mcpID)
  {
    if (sSetLocalMCPs_pattern1.find(mcpID) != sSetLocalMCPs_pattern1.end()) {
      return EPatternType::kPattern1;
    } else if (sSetLocalMCPs_pattern2.find(mcpID) != sSetLocalMCPs_pattern2.end()) {
      return EPatternType::kPattern2;
    } else if (sSetLocalMCPs_pattern3.find(mcpID) != sSetLocalMCPs_pattern3.end()) {
      return EPatternType::kPattern3;
    }
    return EPatternType::kNone;
  }

  static std::array<EPatternType, sNMCPs> getPatternMap()
  {
    std::array<EPatternType, sNMCPs> patternMap{};
    for (int iMCP = 0; iMCP < sNMCPs; iMCP++) {
      patternMap[iMCP] = getPatternType(iMCP);
    }
    return patternMap;
  }

  static const inline std::array<EPatternType, sNMCPs> sPatternMap = getPatternMap();
  using PairToStatus_t = std::map<std::pair<int, int>, EChannelMask>; // first - selected channel, second - neighbour
  // 0-1-3-2 schema
  static const inline PairToStatus_t sMapLocalChPos_pattern1 = {{{0, 1}, EChannelMask::kRight},
                                                                {{0, 2}, EChannelMask::kLeft},
                                                                {{0, 3}, EChannelMask::kDiag},
                                                                {{1, 0}, EChannelMask::kLeft},
                                                                {{1, 2}, EChannelMask::kDiag},
                                                                {{1, 3}, EChannelMask::kRight},
                                                                {{2, 1}, EChannelMask::kDiag},
                                                                {{2, 0}, EChannelMask::kRight},
                                                                {{2, 3}, EChannelMask::kLeft},
                                                                {{3, 1}, EChannelMask::kLeft},
                                                                {{3, 2}, EChannelMask::kRight},
                                                                {{3, 0}, EChannelMask::kDiag}};
  // 0-2-3-1 schema
  static const inline PairToStatus_t sMapLocalChPos_pattern2 = {{{0, 1}, EChannelMask::kLeft},
                                                                {{0, 2}, EChannelMask::kRight},
                                                                {{0, 3}, EChannelMask::kDiag},
                                                                {{1, 0}, EChannelMask::kRight},
                                                                {{1, 2}, EChannelMask::kDiag},
                                                                {{1, 3}, EChannelMask::kLeft},
                                                                {{2, 1}, EChannelMask::kDiag},
                                                                {{2, 0}, EChannelMask::kLeft},
                                                                {{2, 3}, EChannelMask::kRight},
                                                                {{3, 1}, EChannelMask::kRight},
                                                                {{3, 2}, EChannelMask::kLeft},
                                                                {{3, 0}, EChannelMask::kDiag}};
  // 0-3-2-1 schema
  static const inline PairToStatus_t sMapLocalChPos_pattern3 = {{{0, 1}, EChannelMask::kLeft},
                                                                {{0, 2}, EChannelMask::kDiag},
                                                                {{0, 3}, EChannelMask::kRight},
                                                                {{1, 0}, EChannelMask::kRight},
                                                                {{1, 2}, EChannelMask::kLeft},
                                                                {{1, 3}, EChannelMask::kDiag},
                                                                {{2, 1}, EChannelMask::kRight},
                                                                {{2, 0}, EChannelMask::kDiag},
                                                                {{2, 3}, EChannelMask::kLeft},
                                                                {{3, 1}, EChannelMask::kDiag},
                                                                {{3, 2}, EChannelMask::kRight},
                                                                {{3, 0}, EChannelMask::kLeft}};

  static const inline std::map<EPatternType, PairToStatus_t> sMapPatternType = {
    {EPatternType::kPattern1, sMapLocalChPos_pattern1},
    {EPatternType::kPattern2, sMapLocalChPos_pattern2},
    {EPatternType::kPattern3, sMapLocalChPos_pattern3}};
  static const inline std::vector<std::pair<std::set<EChannelMask>, EChannelMask>> sVecCombinations = {
    {{EChannelMask::kRight, EChannelMask::kLeft}, EChannelMask::kLeftRight},
    {{EChannelMask::kDiag, EChannelMask::kLeft}, EChannelMask::kDiagLeft},
    {{EChannelMask::kDiag, EChannelMask::kRight}, EChannelMask::kDiagRight},
    {{EChannelMask::kDiag, EChannelMask::kLeft, EChannelMask::kRight}, EChannelMask::kAll}};
  static EChannelMask combineChannelMask(const std::set<EChannelMask>& setChannelMask)
  {
    if (setChannelMask.size() == 0) {
      return EChannelMask::kNone;
    } else if (setChannelMask.size() == 1) {
      return *setChannelMask.begin();
    } else {
      for (const auto& en : sVecCombinations) {
        if (en.first == setChannelMask) {
          return en.second;
        }
      }
    }
    return EChannelMask::kUnknown;
  }
  using MapChannelMasks_t = std::array<std::array<EChannelMask, sNchPerMCP>, sAllBits + 1>;
  static MapChannelMasks_t getMapChannelMasks(EPatternType patterType)
  {
    MapChannelMasks_t mapResult{{EChannelMask::kUnknown, EChannelMask::kUnknown, EChannelMask::kUnknown, EChannelMask::kUnknown}};
    for (int iStatus = 1; iStatus < sAllBits + 1; iStatus++) {
      for (int iBitFirst = 0; iBitFirst < sNchPerMCP; iBitFirst++) {
        mapResult[iStatus][iBitFirst] = EChannelMask::kUnknown;
        std::set<EChannelMask> setChannelMasks{};
        for (int iBitSecond = 0; iBitSecond < sNchPerMCP; iBitSecond++) {
          const int bitPair = (1 << iBitFirst) | (1 << iBitSecond);
          if (iBitFirst != iBitSecond && ((iStatus & bitPair) == bitPair)) {
            setChannelMasks.insert(sMapPatternType.find(patterType)->second.find({iBitFirst, iBitSecond})->second);
          }
        }
        mapResult[iStatus][iBitFirst] = combineChannelMask(setChannelMasks);
      }
    }
    return mapResult;
  }

  static const inline std::array<MapChannelMasks_t, 3> sMapChannelMasks = {
    getMapChannelMasks(EPatternType::kPattern1),
    getMapChannelMasks(EPatternType::kPattern2),
    getMapChannelMasks(EPatternType::kPattern3)};

  static inline unsigned int getMCPID(unsigned int chID) { return chID / sNchPerMCP; }
  static inline unsigned int getLocalChID(unsigned int chID) { return chID % sNchPerMCP; }
  static inline EPatternType getPatternType(unsigned int mcpID) { return mcpID < sNMCPs ? sPatternMap[mcpID] : EPatternType::kNone; }
  inline void addChannelData(const o2::ft0::ChannelData& channelData)
  {
    const auto localChID = getLocalChID(channelData.ChId);
    mSumAmp += channelData.QTCAmpl;
    mAmp[localChID] = channelData.QTCAmpl;
    mActiveChannels.set(localChID);
  }
  inline std::tuple<int, EChannelMask> getInfo(const o2::ft0::ChannelData& channelData) const
  {
    const auto localChID = getLocalChID(channelData.ChId);
    const auto& refMapChannelMasks = sMapChannelMasks[static_cast<int>(mPatternType)];
    return std::tie(mSumAmp, refMapChannelMasks[mActiveChannels.to_ulong()][localChID]);
  }
  unsigned int mMCPID{sNMCPs};
  EPatternType mPatternType{EPatternType::kNone};
  std::bitset<sNchPerMCP> mActiveChannels{};
  std::array<int, sNchPerMCP> mAmp;
  int mSumAmp{0};
};
} // namespace o2::ft0
#endif