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
 //                   Poznan University of Technology, Poznań, Poland

//===============================================================================================================================================================================================================

#include "xFile.h"
#include "xSeq.h"
#include "xPSNR.h"
#include <math.h>
#include <fstream>
#include <time.h>
#include <limits>
#include <numeric>
#include <cassert>
#include <thread>
#include "lib_fmt/chrono.h"

#if defined(_OPENMP)
#include <omp.h>
#endif //_OPENMP

//===============================================================================================================================================================================================================

int main(int /*argc*/, char** /*argv*/, char** /*envp*/)
{   
  //readed from commandline/config 
  std::string InputFile[2]    = { "../../Poznan_Street/Poznan_Street_00_1920x1088_tex_cam03.yuv", "../../Poznan_Street/Poznan_Street_00_1920x1088_tex_cam03_jpeg.yuv" };
  int32       PictureWidth    = 1920;
  int32       PictureHeight   = 1088;
  int32       BitDepth        = 8   ;  
  int32       ChromaFormat    = 420 ;
  int32       NumberOfFrames  = -1  ;  
  int32       NumberOfThreads = -1  ;
  int32       VerboseLevel    = 4   ;

  //check OpenMP
#if defined(_OPENMP)
  int32 FinalNumberOfThreads = NumberOfThreads < 0 ? omp_get_max_threads() : std::min(NumberOfThreads, omp_get_max_threads());
#else
  int32 FinalNumberOfThreads = 0;
#endif

  //print config
  if(VerboseLevel >= 1)
  {
    fmt::printf("Configuration:\n");
    fmt::printf("InputFile0      = %s\n", InputFile[0]     );
    fmt::printf("InputFile1      = %s\n", InputFile[1]     );
    fmt::printf("PictureWidth    = %d\n", PictureWidth     );
    fmt::printf("PictureHeight   = %d\n", PictureHeight    );
    fmt::printf("BitDepth        = %d\n", BitDepth         );
    fmt::printf("ChromaFormat    = %d\n", ChromaFormat     );
    fmt::printf("NumberOfFrames  = %d%s\n", NumberOfFrames, NumberOfFrames==NOT_VALID ? "  (all)" : "");
    fmt::printf("NumberOfThreads = %d%s\n", FinalNumberOfThreads, NumberOfThreads == NOT_VALID ? "  (all)" : "");
    fmt::printf("VerboseLevel    = %d\n", VerboseLevel     );
    fmt::printf("\n");
  }

  //==============================================================================
  //preparation
  if(VerboseLevel >= 2) { fmt::printf("Initializing:\n"); }

  int64 SizeOfInputFile[2];
  for(int32 i = 0; i < 2; i++)
  {
    if(!xFile::exist(InputFile[i])) { fmt::printf("ERROR --> InputFile does not exist (%s)", InputFile[i]); return EXIT_FAILURE; }
    SizeOfInputFile[i] = xFile::filesize(InputFile[i]);
    if(VerboseLevel >= 1) { fmt::printf("SizeOfInputFile%d = %d\n", i, SizeOfInputFile[i]); }
  }

  int32 NumOfFrames[2];
  for(int32 i = 0; i < 2; i++)
  {
    NumOfFrames[i] = xSeq::calcNumFramesInFile(PictureWidth, PictureHeight, BitDepth, ChromaFormat, SizeOfInputFile[i]);
    if(VerboseLevel >= 1) { fmt::printf("DetectedFrames%d  = %d\n", i, NumOfFrames[i]); }
  }

  int32 MinSeqNumFrames = xMin(NumOfFrames[0], NumOfFrames[1]);
  int32 NumFrames       = NumberOfFrames > 0 ? NumberOfFrames : MinSeqNumFrames;
  if(VerboseLevel >= 1) { fmt::printf("FramesToProcess  = %d\n", NumFrames); }
  fmt::printf("\n");

  xSeq Sequence[2];
  for(int32 i = 0; i < 2; i++) { Sequence[i].create(PictureWidth, PictureHeight, BitDepth, ChromaFormat); }
  xPic PictureP[2];
  for(int32 i = 0; i < 2; i++) { PictureP[i].create(PictureWidth, PictureHeight, BitDepth, false); }

  for(int32 i = 0; i < 2; i++)
  {
    bool OpenSucces = Sequence[i].openFile(InputFile[i], xSeq::eMode::Read);
    if(!OpenSucces) { fmt::printf("ERROR --> InputFile openinf failure (%s)", InputFile[i]); return EXIT_FAILURE; }
  }

  xPSNR Processor;
  Processor.setNumThreads  (FinalNumberOfThreads);
  Processor.setVerboseLevel(VerboseLevel        );

  //==============================================================================
  //running
  if (VerboseLevel >= 2) { fmt::printf("Running:\n"); }

  tDuration DurationLoad = tDuration(0);
  tDuration DurationPSNR = tDuration(0);

  std::vector<flt64> FramePSNR[4];
  for(int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++) { FramePSNR[CmpIdx].resize(NumFrames); }

  bool AllExact = true;
  bool AnyFake  = false;
  for(int32 f = 0; f < NumFrames; f++)
  {
    tTimePoint T0 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    //LOAD
    bool ReadOK[2] = { true, true };
    for(int32 i = 0; i < 2; i++)
    {
      bool Result = Sequence[i].readFrame(&(PictureP[i]));
      ReadOK[i] = Result;
      PictureP[i].extend();
    }
    for(int32 i = 0; i < 2; i++) { if(!ReadOK[i]) { fmt::printf("ERROR --> InputFile read error (%s)", InputFile[i]); return EXIT_FAILURE; } }

    tTimePoint T1 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    //CALC PSNR
    const auto [PSNR, Exact] = Processor.calcPicPSNR(PictureP[0], PictureP[1]);

    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      if(Exact[CmpIdx]) { AnyFake  = true ; }
      else              { AllExact = false; }
      FramePSNR[CmpIdx][f] = PSNR[CmpIdx];        
    }

    if(VerboseLevel >= 2)
    {
      fmt::printf("Frame %08d   PSNR %8.4f %8.4f %8.4f", f, PSNR[0], PSNR[1], PSNR[2]);
      if(Exact[0]) { fmt::printf(" ExactY"); }
      if(Exact[1]) { fmt::printf(" ExactU"); }
      if(Exact[2]) { fmt::printf(" ExactV"); }
      fmt::printf("\n");
    }

    tTimePoint T2 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    DurationLoad += (T1 - T0);
    DurationPSNR += (T2 - T1);
  }

  flt64V4 SumPSNR = xMakeVec4(0.0);

  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  {
    SumPSNR[CmpIdx] = xPSNR::Accumulate(FramePSNR[CmpIdx]);
  }

  flt64V4 AvgPSNR = SumPSNR / NumFrames;

  //==============================================================================
  //printout results
  fmt::printf("\n\n");
  fmt::printf("Average          PSNR %8.4f dB  %8.4f dB  %8.4f db\n", AvgPSNR[0], AvgPSNR[1], AvgPSNR[2]);
  fmt::printf("\n");
  if(AnyFake ) { fmt::printf("FakePSNR\n"); }
  if(AllExact) { fmt::printf("ExactSequences\n"); }
  fmt::printf("\n");
  if(VerboseLevel >= 3)
  {
    fmt::printf("AvgTime          LOAD %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationLoad).count() / NumFrames); 
    fmt::printf("AvgTime          PSNR %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationPSNR).count() / NumFrames); 
  }
  fmt::printf("\n");
  fmt::printf("NumFrames %d\n", NumFrames);
  fmt::printf("END-OF-LOG\n");

  return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================
