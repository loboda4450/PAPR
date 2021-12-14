/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (CmpIdx) 2010-2020, ISO/IEC
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
 //                   Poznan University of Technology, Poznań, Poland

#include "xPSNR.h"
#include <cassert>
#include <numeric>
#include <mutex>
#include <atomic>

//===============================================================================================================================================================================================================
// xPSNR
//===============================================================================================================================================================================================================
xPSNR::tRes4 xPSNR::calcPicPSNR(const xPic& Tst, const xPic& Ref)
{
  assert(Ref.getWidth() == Tst.getWidth() && Ref.getHeight() == Tst.getHeight() && Ref.getStride() == Tst.getStride());

  flt64V4 PSNR  = xMakeVec4(flt64_max );
  boolV4  Exact = xMakeVec4(false     );

  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  {
    std::tie(PSNR[CmpIdx], Exact[CmpIdx]) = xCalcCmpPSNR_BUFFER(Tst, Ref, CmpIdx);
  }

  return std::make_tuple(PSNR, Exact);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

xPSNR::tRes1 xPSNR::xCalcCmpPSNR_BASE(const xPic& Tst, const xPic& Ref, int32 CmpIdx)
{
  assert(Ref.getWidth() == Tst.getWidth() && Ref.getHeight() == Tst.getHeight() && Ref.getStride() == Tst.getStride() && CmpIdx >= 0 && CmpIdx < 3);

  const int32   Width     = Ref.getWidth ();
  const int32   Height    = Ref.getHeight();
  const uint16* TstPtr    = Tst.getAddr  (CmpIdx);
  const uint16* RefPtr    = Ref.getAddr  (CmpIdx);
  const int32   TstStride = Tst.getStride();
  const int32   RefStride = Ref.getStride();

  uint64 FrameDistortion = 0;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      FrameDistortion += (uint64)xPow2(((int32)TstPtr[x]) - ((int32)RefPtr[x]));
    }
    TstPtr += TstStride;
    RefPtr += RefStride;
  }

  flt64 PSNR  = xCalcPSNR((flt64)FrameDistortion, Tst.getArea(), Tst.getBitDepth());
  bool  Exact = FrameDistortion == 0;
  if(Exact)
  {
    flt64 FakePSNR = xCalcPSNR(1, Tst.getArea(), Tst.getBitDepth());
    PSNR  = FakePSNR;
  }
  return std::make_tuple(PSNR, Exact);
}
xPSNR::tRes1 xPSNR::xCalcCmpPSNR_OMP(const xPic& Tst, const xPic& Ref, int32 CmpIdx)
{
  assert(Ref.getWidth() == Tst.getWidth() && Ref.getHeight() == Tst.getHeight() && Ref.getStride() == Tst.getStride() && CmpIdx >= 0 && CmpIdx < 3);
    const int32   Width     = Ref.getWidth ();
    const int32   Height    = Ref.getHeight();
    const uint16* TstPtr    = Tst.getAddr  (CmpIdx);
    const uint16* RefPtr    = Ref.getAddr  (CmpIdx);
    const int32   TstStride = Tst.getStride();
    const int32   RefStride = Ref.getStride();


    uint64 FrameDistortion = 0;

    #pragma omp parallel for schedule(static) if(m_NumThreads)
    for(int32 y = 0; y < Height; y++)
    {
        const uint16* tmpptr = TstPtr + TstStride*y;
        const uint16* tmpptrref = RefPtr + RefStride*y;

        for(int32 x = 0; x < Width; x++)
        {
            FrameDistortion += (uint64)xPow2(((int32)tmpptr[x]) - ((int32)tmpptrref[x]));
        }
        TstPtr += TstStride;
        RefPtr += RefStride;
    }

    flt64 PSNR  = xCalcPSNR((flt64)FrameDistortion, Tst.getArea(), Tst.getBitDepth());
    bool  Exact = FrameDistortion == 0;
    if(Exact)
    {
        flt64 FakePSNR = xCalcPSNR(1, Tst.getArea(), Tst.getBitDepth());
        PSNR  = FakePSNR;
    }
    return std::make_tuple(PSNR, Exact);
}
xPSNR::tRes1 xPSNR::xCalcCmpPSNR_MUTEX(const xPic& Tst, const xPic& Ref, int32 CmpIdx)
{
  assert(Ref.getWidth() == Tst.getWidth() && Ref.getHeight() == Tst.getHeight() && Ref.getStride() == Tst.getStride() && CmpIdx >= 0 && CmpIdx < 3);
    const int32   Width     = Ref.getWidth ();
    const int32   Height    = Ref.getHeight();
    const uint16* TstPtr    = Tst.getAddr  (CmpIdx);
    const uint16* RefPtr    = Ref.getAddr  (CmpIdx);
    const int32   TstStride = Tst.getStride();
    const int32   RefStride = Ref.getStride();
    std::mutex M;


    uint64 FrameDistortion = 0;

    #pragma omp parallel for schedule(static) if(m_NumThreads)
    for(int32 y = 0; y < Height; y++)
    {
        const uint16* tmpptr = TstPtr + TstStride*y;
        const uint16* tmpptrref = RefPtr + RefStride*y;

        for(int32 x = 0; x < Width; x++)
        {
            M.lock();
            FrameDistortion += (uint64)xPow2(((int32)tmpptr[x]) - ((int32)tmpptrref[x]));
            M.unlock();
        }
        TstPtr += TstStride;
        RefPtr += RefStride;
    }

    flt64 PSNR  = xCalcPSNR((flt64)FrameDistortion, Tst.getArea(), Tst.getBitDepth());
    bool  Exact = FrameDistortion == 0;
    if(Exact)
    {
        flt64 FakePSNR = xCalcPSNR(1, Tst.getArea(), Tst.getBitDepth());
        PSNR  = FakePSNR;
    }
    return std::make_tuple(PSNR, Exact);
}
xPSNR::tRes1 xPSNR::xCalcCmpPSNR_ATOMIC(const xPic& Tst, const xPic& Ref, int32 CmpIdx)
{
    assert(Ref.getWidth() == Tst.getWidth() && Ref.getHeight() == Tst.getHeight() && Ref.getStride() == Tst.getStride() && CmpIdx >= 0 && CmpIdx < 3);
    const int32   Width     = Ref.getWidth ();
    const int32   Height    = Ref.getHeight();
    const uint16* TstPtr    = Tst.getAddr  (CmpIdx);
    const uint16* RefPtr    = Ref.getAddr  (CmpIdx);
    const int32   TstStride = Tst.getStride();
    const int32   RefStride = Ref.getStride();
    std::mutex M;


    std::atomic_uint64_t FrameDistortion = 0;

#pragma omp parallel for schedule(static) if(m_NumThreads)
    for(int32 y = 0; y < Height; y++)
    {
        const uint16* tmpptr = TstPtr + TstStride*y;
        const uint16* tmpptrref = RefPtr + RefStride*y;

        for(int32 x = 0; x < Width; x++)
        {
            FrameDistortion += (uint64)xPow2(((int32)tmpptr[x]) - ((int32)tmpptrref[x]));
        }
        TstPtr += TstStride;
        RefPtr += RefStride;
    }

    flt64 PSNR  = xCalcPSNR((flt64)FrameDistortion, Tst.getArea(), Tst.getBitDepth());
    bool  Exact = FrameDistortion == 0;
    if(Exact)
    {
        flt64 FakePSNR = xCalcPSNR(1, Tst.getArea(), Tst.getBitDepth());
        PSNR  = FakePSNR;
    }
    return std::make_tuple(PSNR, Exact);
}
xPSNR::tRes1 xPSNR::xCalcCmpPSNR_BUFFER(const xPic& Tst, const xPic& Ref, int32 CmpIdx)
{
    assert(Ref.getWidth() == Tst.getWidth() && Ref.getHeight() == Tst.getHeight() && Ref.getStride() == Tst.getStride() && CmpIdx >= 0 && CmpIdx < 3);

    const int32   Width     = Ref.getWidth ();
    const int32   Height    = Ref.getHeight();
    const uint16* TstPtr    = Tst.getAddr  (CmpIdx);
    const uint16* RefPtr    = Ref.getAddr  (CmpIdx);
    const int32   TstStride = Tst.getStride();
    const int32   RefStride = Ref.getStride();
    uint64* FrameDistortion = new uint64[Height];
    uint64 SumOfFrameDistortion = 0;

    for(int i=0; i < Height; i++){
        // initialize array with 0's
        FrameDistortion[i] = 0;
    }

    for(int32 y = 0; y < Height; y++)
    {
        for(int32 x = 0; x < Width; x++)
        {
            FrameDistortion[y] += (uint64)xPow2(((int32)TstPtr[x]) - ((int32)RefPtr[x]));
        }
        TstPtr += TstStride;
        RefPtr += RefStride;
    }

    for(int i=0; i < Height; i++){
        // sum array, too lazy to use accumulate
        SumOfFrameDistortion += FrameDistortion[i];
    }

    flt64 PSNR  = xCalcPSNR((flt64)SumOfFrameDistortion, Tst.getArea(), Tst.getBitDepth());
    bool  Exact = FrameDistortion == 0;
    if(Exact)
    {
        flt64 FakePSNR = xCalcPSNR(1, Tst.getArea(), Tst.getBitDepth());
        PSNR  = FakePSNR;
    }
    return std::make_tuple(PSNR, Exact);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64 xPSNR::xCalcPSNR(flt64 SSD, int32 Area, int32 BitDepth)
{
  uint64 NumPoints = Area;
  uint64 MaxValue  = xBitDepth2MaxValue(BitDepth);
  uint64 MAX       = (NumPoints)*xPow2(MaxValue);
  flt64  PSNR      = SSD > 0 ? 10.0 * log10((flt64)MAX / SSD) : flt64_max;
  return PSNR;
}

flt64 xPSNR::KahanandBabuskaNeumaierSumation(const flt64* x, const int32 n)
{
  if(n == 0) { return 0; }

  flt64 s = x[0];
  flt64 c = 0;
  for(int32 i = 1; i < n; i++)
  {
    flt64 t = s + x[i];
    if(xAbs(s) >= xAbs(x[i])) { c += ((s - t) + x[i]); }
    else                      { c += ((x[i] - t) + s); }
    s = t;
  }
  return s + c;
}

//===============================================================================================================================================================================================================
