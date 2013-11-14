/*
 * Copyright (c) 2013 National Instruments Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <fcntl.h>
#include <unistd.h>

extern "C" {
#include "lms2012.h"
// #include "bytecodes.h"
#include "c_sound.h"
}

#include "ExecutionContext.h"
#include "StringUtilities.h"

using namespace Vireo;


VIVM_FUNCTION_SIGNATURE0(SoundStop)
{
    /*
    IMGDATA ByteCode[] = { opSOUND, LC0(BREAK), opOBJECT_END };
    ExecuteByteCode(ByteCode,NULL,NULL);
    */

    // Stop sound
    SoundInstance.cSoundState = SOUND_STOPPED;
    if (SoundInstance.hSoundFile >= 0)
    {
        close(SoundInstance.hSoundFile);
        SoundInstance.hSoundFile = -1;
    }

    // Assemble SoundData
    const UInt8 BytesToWrite = 1;
    DATA8 SoundData[BytesToWrite];
    SoundData[0] = BREAK;

    // Write to sound device file
    SoundInstance.SoundDriverDescriptor = open(SOUND_DEVICE_NAME, O_WRONLY);
    if (SoundInstance.SoundDriverDescriptor >= 0)
    {
        write(SoundInstance.SoundDriverDescriptor, SoundData, BytesToWrite);
        close(SoundInstance.SoundDriverDescriptor);
        SoundInstance.SoundDriverDescriptor = -1;
    }
    else
        SoundInstance.cSoundState = SOUND_STOPPED;

    return _NextInstruction();
}

VIVM_FUNCTION_SIGNATURE3(SoundTone, UInt8, UInt16, UInt16)
{
    UInt8  Volume    = _Param(0);
    UInt16 Frequency = _Param(1);
    UInt16 Duration  = _Param(2);

    Volume = Volume > 0   ? Volume : 0;
    Volume = Volume < 100 ? Volume : 100;

    /*
    IMGDATA ByteCode[] = { opSOUND, LC0(TONE), LC0(Volume), LC2(Frequency), LC2(Duration), opOBJECT_END };
    ExecuteByteCode(ByteCode,NULL,NULL);
    */

    // Change state
    (*SoundInstance.pSound).Status = BUSY;
    SoundInstance.cSoundState = SOUND_TONE_PLAYING;
    SoundInstance.SoundOwner = CallingObjectId();

    // Assemble SoundData
    const UInt8 BytesToWrite = 6;
    DATA8 SoundData[BytesToWrite];
    SoundData[0] = TONE;
    SoundData[1] = (Volume + 7) / 8;
    SoundData[2] = (UBYTE) Frequency;
    SoundData[3] = (UBYTE) (Frequency >> 8);
    SoundData[4] = (UBYTE) Duration;
    SoundData[5] = (UBYTE) (Duration >> 8);

    // Write to sound device file
    SoundInstance.SoundDriverDescriptor = open(SOUND_DEVICE_NAME, O_WRONLY);
    if (SoundInstance.SoundDriverDescriptor >= 0)
    {
        write(SoundInstance.SoundDriverDescriptor, SoundData, BytesToWrite);
        close(SoundInstance.SoundDriverDescriptor);
        SoundInstance.SoundDriverDescriptor = -1;
    }
    else
        SoundInstance.cSoundState = SOUND_STOPPED;

    return _NextInstruction();
}

VIVM_FUNCTION_SIGNATURE2(SoundPlay, UInt8, Utf8String*)
{
    UInt8       Volume   = _Param(0);
    Utf8String *fileName = _Param(1);

    Volume = Volume > 0   ? Volume : 0;
    Volume = Volume < 100 ? Volume : 100;

    /*
    IMGDATA BCStart[] = { opSOUND, LC0(PLAY), LC0(Volume), LCS};
    IMGDATA BCEnd[]   = { opOBJECT_END };
    IMGDATA *ByteCode = (IMGDATA *) new char[sizeof(BCStart) + fileName->Length() + 1 + sizeof(BCEnd)];

    // Form the array Bytecode by concatenating BCStart, the null-terminated fileName, and BCEnd.
    memcpy(ByteCode, BCStart, sizeof(BCStart));
    memcpy(ByteCode+sizeof(BCStart), fileName->Begin(), fileName->Length()+1);
    memcpy(ByteCode+sizeof(BCStart)+fileName->Length()+1, BCEnd, sizeof(BCEnd));

    ExecuteByteCode(ByteCode,NULL,NULL);
    delete [] ByteCode;
    */

    SoundInstance.cSoundState = SOUND_STOPPED;
    if (SoundInstance.hSoundFile >= 0)
    {
        close(SoundInstance.hSoundFile);
        SoundInstance.hSoundFile = -1;
    }
    (*SoundInstance.pSound).Status = BUSY;
    SoundInstance.SoundOwner = CallingObjectId();

    // Assemble SoundData
    const UInt8 BytesToWrite = 2;
    DATA8 SoundData[BytesToWrite];
    SoundData[0] = PLAY;
    SoundData[1] = (Volume + 7) / 8;

    // Handle fileName
    char pathName[MAX_FILENAME_SIZE];
    UBYTE Tmp1;
    UBYTE Tmp2;

    if (fileName->Begin())
    {
        pathName[0] = 0;
        if (*((Utf8Char *) fileName->Begin()) != '.')
        {
            GetResourcePath(pathName, MAX_FILENAME_SIZE);
            sprintf(SoundInstance.PathBuffer, "%s%s.rsf", pathName, fileName->Begin());
        }
        else
            sprintf(SoundInstance.PathBuffer, "%s.rsf", fileName->Begin());

        SoundInstance.hSoundFile = open(SoundInstance.PathBuffer, O_RDONLY, 0666);
        if (SoundInstance.hSoundFile >= 0)
        {
            stat(SoundInstance.PathBuffer,&SoundInstance.FileStatus);
            SoundInstance.SoundFileLength = SoundInstance.FileStatus.st_size;

            read(SoundInstance.hSoundFile, &Tmp1, 1);
            read(SoundInstance.hSoundFile, &Tmp2, 1);
            SoundInstance.SoundFileFormat = (UWORD)Tmp1 << 8 | (UWORD)Tmp2;

            read(SoundInstance.hSoundFile, &Tmp1, 1);
            read(SoundInstance.hSoundFile, &Tmp2, 1);
            SoundInstance.SoundDataLength = (UWORD)Tmp1 << 8 | (UWORD)Tmp2;

            read(SoundInstance.hSoundFile, &Tmp1, 1);
            read(SoundInstance.hSoundFile, &Tmp2, 1);
            SoundInstance.SoundSampleRate = (UWORD)Tmp1 << 8 | (UWORD)Tmp2;

            read(SoundInstance.hSoundFile, &Tmp1, 1);
            read(SoundInstance.hSoundFile, &Tmp2, 1);
            SoundInstance.SoundPlayMode = (UWORD)Tmp1 << 8 | (UWORD)Tmp2;

            SoundInstance.cSoundState = SOUND_SETUP_FILE;

            if (SoundInstance.SoundFileFormat == FILEFORMAT_ADPCM_SOUND)
                cSoundInitAdPcm();
        }
    }

    SoundInstance.SoundDriverDescriptor = open(SOUND_DEVICE_NAME, O_WRONLY);
    if (SoundInstance.SoundDriverDescriptor >= 0)
    {
        write(SoundInstance.SoundDriverDescriptor, SoundData, BytesToWrite);
        close(SoundInstance.SoundDriverDescriptor);
        SoundInstance.SoundDriverDescriptor = -1;
        SoundInstance.cSoundState = SOUND_FILE_PLAYING;
    }
    else
        SoundInstance.cSoundState = SOUND_STOPPED;

    return _NextInstruction();
}

VIVM_FUNCTION_SIGNATURE2(SoundPlayLoop, UInt8, Utf8String*)
{
    UInt8       Volume   = _Param(0);
    Utf8String *fileName = _Param(1);

    Volume = Volume > 0   ? Volume : 0;
    Volume = Volume < 100 ? Volume : 100;

    /*
    IMGDATA BCStart[] = { opSOUND, LC0(REPEAT), LC0(Volume), LCS};
    IMGDATA BCEnd[]   = { opOBJECT_END };
    IMGDATA *ByteCode = new IMGDATA[sizeof(BCStart) + fileName->Length() + 1 + sizeof(BCEnd)];

    // Form the array Bytecode by concatenating BCStart, the null-terminated fileName, and BCEnd.
    memcpy(ByteCode, BCStart, sizeof(BCStart));
    memcpy(ByteCode+sizeof(BCStart), fileName->Begin(), fileName->Length()+1);
    memcpy(ByteCode+sizeof(BCStart)+fileName->Length()+1, BCEnd, sizeof(BCEnd));

    ExecuteByteCode(ByteCode,NULL,NULL);
    delete [] ByteCode;
    */

    SoundInstance.cSoundState = SOUND_STOPPED;
    if (SoundInstance.hSoundFile >= 0)
    {
        close(SoundInstance.hSoundFile);
        SoundInstance.hSoundFile = -1;
    }
    (*SoundInstance.pSound).Status = BUSY;
    SoundInstance.SoundOwner = CallingObjectId();

    // Assemble SoundData
    const UInt8 BytesToWrite = 2;
    DATA8 SoundData[BytesToWrite];
    SoundData[0] = PLAY;
    SoundData[1] = (Volume + 7) / 8;

    // Handle fileName
    char pathName[MAX_FILENAME_SIZE];
    UBYTE Tmp1;
    UBYTE Tmp2;

    if (fileName->Begin())
    {
        pathName[0] = 0;
        if (*((Utf8Char *) fileName->Begin()) != '.')
        {
            GetResourcePath(pathName, MAX_FILENAME_SIZE);
            sprintf(SoundInstance.PathBuffer, "%s%s.rsf", pathName, fileName->Begin());
        }
        else
            sprintf(SoundInstance.PathBuffer, "%s.rsf", fileName->Begin());

        SoundInstance.hSoundFile = open(SoundInstance.PathBuffer, O_RDONLY, 0666);
        if (SoundInstance.hSoundFile >= 0)
        {
            stat(SoundInstance.PathBuffer,&SoundInstance.FileStatus);
            SoundInstance.SoundFileLength = SoundInstance.FileStatus.st_size;

            read(SoundInstance.hSoundFile, &Tmp1, 1);
            read(SoundInstance.hSoundFile, &Tmp2, 1);
            SoundInstance.SoundFileFormat = (UWORD)Tmp1 << 8 | (UWORD)Tmp2;

            read(SoundInstance.hSoundFile, &Tmp1, 1);
            read(SoundInstance.hSoundFile, &Tmp2, 1);
            SoundInstance.SoundDataLength = (UWORD)Tmp1 << 8 | (UWORD)Tmp2;

            read(SoundInstance.hSoundFile, &Tmp1, 1);
            read(SoundInstance.hSoundFile, &Tmp2, 1);
            SoundInstance.SoundSampleRate = (UWORD)Tmp1 << 8 | (UWORD)Tmp2;

            read(SoundInstance.hSoundFile, &Tmp1, 1);
            read(SoundInstance.hSoundFile, &Tmp2, 1);
            SoundInstance.SoundPlayMode = (UWORD)Tmp1 << 8 | (UWORD)Tmp2;

            SoundInstance.cSoundState = SOUND_SETUP_FILE;

            if (SoundInstance.SoundFileFormat == FILEFORMAT_ADPCM_SOUND)
                cSoundInitAdPcm();
        }
    }

    SoundInstance.SoundDriverDescriptor = open(SOUND_DEVICE_NAME, O_WRONLY);
    if (SoundInstance.SoundDriverDescriptor >= 0)
    {
        write(SoundInstance.SoundDriverDescriptor, SoundData, BytesToWrite);
        close(SoundInstance.SoundDriverDescriptor);
        SoundInstance.SoundDriverDescriptor = -1;
        SoundInstance.cSoundState = SOUND_FILE_LOOPING;
    }
    else
        SoundInstance.cSoundState = SOUND_STOPPED;

    return _NextInstruction();
}

VIVM_FUNCTION_SIGNATURE1(SoundTest, UInt8)
{
    /*
    UInt8 isBusy = _Param(0); // reference

    IMGDATA ByteCode[] = { opSOUND_TEST, LV0(0), opOBJECT_END };
    ExecuteByteCode(ByteCode,NULL,(VARDATA *) &isBusy);
    */

    _Param(0) = ((*SoundInstance.pSound).Status == BUSY);

    return _NextInstruction();
}

#include "TypeDefiner.h"
VIREO_DEFINE_BEGIN(EV3_IO)
    // Types
    //VIREO_DEFINE_TYPE(FileHandle, ".DataPointer")
    // Values
    VIREO_DEFINE_FUNCTION(SoundStop, "p()");
    VIREO_DEFINE_FUNCTION(SoundTone, "p(i(.UInt8),i(.UInt16),i(.UInt16))");
    VIREO_DEFINE_FUNCTION(SoundPlay, "p(i(.UInt8),i(.Utf8String))");
    VIREO_DEFINE_FUNCTION(SoundPlayLoop, "p(i(.UInt8),i(.Utf8String))");
    VIREO_DEFINE_FUNCTION(SoundTest,"p(o(.UInt8))");
VIREO_DEFINE_END()

