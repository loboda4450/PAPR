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
#include "xPic.h"
#include "xSeq.h"
#include "xSepia.h"
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

int32 main(int /*argc*/, char ** /*argv*/, char ** /*envp*/) {
    //readed from commandline/config
    std::string InputFile = "/tmp/Poznan_Street/Poznan_Street_00_1920x1088_tex_cam03.yuv";
    std::string OutputFile = "/tmp/Poznan_Street/Poznan_Street_00_1920x1088_tex_cam03_test.yuv";
    int32 PictureWidth = 1920;
    int32 PictureHeight = 1088;
    int32 BitDepth = 8;
    int32 ChromaFormat = 420;
    int32 NumberOfFrames = -1;
    int32 NumberOfThreads = -1;
    int32 VerboseLevel = 4;

    //check OpenMP
#if defined(_OPENMP)
    int32 FinalNumberOfThreads =
            NumberOfThreads < 0 ? omp_get_max_threads() : std::min(NumberOfThreads, omp_get_max_threads());
#else
    int32 FinalNumberOfThreads = 0;
#endif

    //print config
    if (VerboseLevel >= 1) {
        fmt::printf("Configuration:\n");
        fmt::printf("InputFile       = %s\n", InputFile);
        fmt::printf("OutputFile      = %s\n", InputFile);
        fmt::printf("PictureWidth    = %d\n", PictureWidth);
        fmt::printf("PictureHeight   = %d\n", PictureHeight);
        fmt::printf("BitDepth        = %d\n", BitDepth);
        fmt::printf("ChromaFormat    = %d\n", ChromaFormat);
        fmt::printf("NumberOfFrames  = %d%s\n", NumberOfFrames, NumberOfFrames == NOT_VALID ? "  (all)" : "");
        fmt::printf("NumberOfThreads = %d%s\n", FinalNumberOfThreads, NumberOfThreads == NOT_VALID ? "  (all)" : "");
        fmt::printf("VerboseLevel    = %d\n", VerboseLevel);
        fmt::printf("\n");
    }

    //==============================================================================
    //operation
    if (VerboseLevel >= 2) { fmt::printf("Initializing:\n"); }

    if (!xFile::exist(InputFile)) {
        fmt::printf("ERROR --> InputFile does not exist (%s)", InputFile);
        return EXIT_FAILURE;
    }
    int64 SizeOfInputFile = xFile::filesize(InputFile);
    if (VerboseLevel >= 1) { fmt::printf("SizeOfInputFile = %d\n", SizeOfInputFile); }


    int32 NumOfFrames = xSeq::calcNumFramesInFile(PictureWidth, PictureHeight, BitDepth, ChromaFormat, SizeOfInputFile);
    if (VerboseLevel >= 1) { fmt::printf("DetectedFrames  = %d\n", NumOfFrames); }

    int32 MinSeqNumFrames = NumOfFrames;
    int32 NumFrames = NumberOfFrames > 0 ? NumberOfFrames : MinSeqNumFrames;
    if (VerboseLevel >= 1) { fmt::printf("FramesToProcess = %d\n", NumFrames); }
    fmt::printf("\n");

    xSeq SequenceSrc(PictureWidth, PictureHeight, BitDepth, ChromaFormat);
    xSeq SequenceDst(PictureWidth, PictureHeight, BitDepth, ChromaFormat);
    SequenceSrc.openFile(InputFile, xSeq::eMode::Read);
    SequenceDst.openFile(OutputFile, xSeq::eMode::Write);

    xPic PictureSrcYUV(PictureWidth, PictureHeight, BitDepth, false);
    xPic PictureDstYUV(PictureWidth, PictureHeight, BitDepth, false);

//    xSepia_STD Processor;
    xSepia_SSE Processor;
    Processor.setNumThreads(FinalNumberOfThreads);
    Processor.setVerboseLevel(VerboseLevel);

    //==============================================================================
    //running
    if (VerboseLevel >= 2) { fmt::printf("Running:\n"); }

    tDuration DurationLoad = tDuration(0);
    tDuration DurationProc = tDuration(0);
    tDuration DurationStor = tDuration(0);

    for (int32 f = 0; f < NumFrames; f++) {
        tTimePoint T0 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

        //LOAD
        bool ReadOK = SequenceSrc.readFrame(&PictureSrcYUV);
        if (!ReadOK) {
            fmt::printf("ERROR --> InputFile read error (%s)", InputFile);
            return EXIT_FAILURE;
        }

        tTimePoint T1 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

        //PROCESS
//        Processor.testCopyContent(PictureDstYUV, PictureSrcYUV);
//        Processor.testYUVtoRGBtoYUV_FLT(PictureDstYUV, PictureSrcYUV);
        Processor.testYUVtoRGBtoYUV_INT(PictureDstYUV, PictureSrcYUV);
//        Processor.applySepiaEffect_FLT(PictureDstYUV, PictureSrcYUV);
//        Processor.applySepiaEffect_INT(PictureDstYUV, PictureSrcYUV);

        tTimePoint T2 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

        //STORE
        bool WriteOK = SequenceDst.writeFrame(&PictureDstYUV);
        if (!WriteOK) {
            fmt::printf("ERROR --> OutputFile write error (%s)", OutputFile);
            return EXIT_FAILURE;
        }

        tTimePoint T3 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

        DurationLoad += (T1 - T0);
        DurationProc += (T2 - T1);
        DurationStor += (T3 - T2);

        if (VerboseLevel >= 2) { fmt::printf("Frame %08d\n", f); }
        if (f==20){break;}
    }

    //==============================================================================
    //cleanup
    SequenceSrc.closeFile();
    SequenceDst.closeFile();

    //==============================================================================
    //printout results
    fmt::printf("\n\n");
    if (VerboseLevel >= 3) {
        fmt::printf("AvgTime LOAD %9.2f ms\n",
                    std::chrono::duration_cast<tDurationMS>(DurationLoad).count() / NumFrames);
        fmt::printf("AvgTime PROC %9.2f ms\n",
                    std::chrono::duration_cast<tDurationMS>(DurationProc).count() / NumFrames);
        fmt::printf("AvgTime STOR %9.2f ms\n",
                    std::chrono::duration_cast<tDurationMS>(DurationStor).count() / NumFrames);
    }
    fmt::printf("\n");
    fmt::printf("NumFrames %d\n", NumFrames);
    fmt::printf("END-OF-LOG\n");

    return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================
