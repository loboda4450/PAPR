#pragma once

/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2020, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

 // Original authors: Jakub Stankowski, jakub.stankowski@put.poznan.pl,
 //                   Adrian Dziembowski, adrian.dziembowski@put.poznan.pl,
 //                   Poznan University of Technology, Poznañ, Poland

#include "CommonDef.h"
#include "xPic.h"
#include "xVec4.h"
#include <numeric>
#include <vector>
#include <tuple>

//===============================================================================================================================================================================================================

class xPSNR
{
public:
  static constexpr bool c_UseKBNS = USE_KBNS;

  using tRes4 = std::tuple<flt64V4, boolV4>;
  using tRes1 = std::tuple<flt64  , bool  >;

protected:
  int32 m_NumThreads    = 0;
  int32 m_VerboseLevel  = 0;
  int32 m_NumComponents = 3;

public:
  void  setNumThreads  (int32 NumThreads  ) { m_NumThreads   = NumThreads  ; }
  void  setVerboseLevel(int32 VerboseLevel) { m_VerboseLevel = VerboseLevel; }
  tRes4 calcPicPSNR(const xPic& Tst, const xPic& Ref);

protected:
  tRes1 xCalcCmpPSNR_BASE  (const xPic& Tst, const xPic& Ref, int32 CmpIdx);
  tRes1 xCalcCmpPSNR_OMP   (const xPic& Tst, const xPic& Ref, int32 CmpIdx);
  tRes1 xCalcCmpPSNR_MUTEX (const xPic& Tst, const xPic& Ref, int32 CmpIdx);
  tRes1 xCalcCmpPSNR_ATOMIC(const xPic& Tst, const xPic& Ref, int32 CmpIdx);
  tRes1 xCalcCmpPSNR_BUFFER(const xPic& Tst, const xPic& Ref, int32 CmpIdx);


  static flt64 xCalcPSNR(flt64 SSD, int32 Area, int32 BitDepth);

public:
  static inline flt64 Accumulate(std::vector<flt64>& Data)
  {
    if constexpr(c_UseKBNS) { return KahanandBabuskaNeumaierSumation(Data.data(), (int32)Data.size()); }
    else                    { return std::accumulate(Data.begin(), Data.end(), (flt64)0);             }
  }
  static flt64 KahanandBabuskaNeumaierSumation(const flt64* x, const int32 n);
};

//===============================================================================================================================================================================================================





