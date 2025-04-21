 /*
  * Copyright (c) 2003 Bill Keirstead
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions
  * are met:
  *
  * 1. Redistributions of source code must retain the above copyright
  *    notice, this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright
  *    notice, this list of conditions and the following disclaimer in the
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
  * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
  */


/********************************************************************
    encodePackets.cpp                                       //WCK025
********************************************************************/

#include "encodePackets.h"
#include "DiffieHellman.h"
#include "encodePackets.h"
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <time.h>

const int nPrime     = 32717;
const int nGenerator = 18334;
const unsigned char szPacketCheck1[] = { 23, 34, 232, 45, 34, 53, 28, 214 };
const unsigned char szPacketCheck2[] = { 40, 92, 34, 234, 4,  82, 8, 42 };
const int nNumRounds = 1;

static void init();
static int getMSBytePos();
static int getLSBytePos();


/********************************************************************
********************************************************************/
void getPublicKey( unsigned char* szPublicKey, unsigned char* szPrivateKey )
{
    int nMostSignificant = getMSBytePos();
    int nLeastSignificant = getLSBytePos();

    for ( int nIndex = 0; nIndex < 4; ++nIndex ) {
        CDiffieHellman dh( nPrime, nGenerator );
        int nPublicKey = dh.getPublicKey();
        int nPrivateKey = dh.getPrivateKey();

        szPrivateKey[nIndex * 2] = ((char*)&nPrivateKey)[nLeastSignificant];
        szPrivateKey[(nIndex * 2) + 1] = ((char*)&nPrivateKey)[nMostSignificant];

        szPublicKey[nIndex * 2] = ((char*)&nPublicKey)[nLeastSignificant];
        szPublicKey[(nIndex * 2) + 1] = ((char*)&nPublicKey)[nMostSignificant];
    }
}


/********************************************************************
********************************************************************/
void getSessionKey( unsigned char* szSessionKey, unsigned char* szPrivateKey, unsigned char* szPublicKey )
{
    int nMostSignificant = getMSBytePos();
    int nLeastSignificant = getLSBytePos();

    for ( int nIndex = 0; nIndex < 4; ++nIndex ) {
        CDiffieHellman dh( nPrime, nGenerator );

        int nPrivateKey = 0;
        int nPublicKey = 0;
        int nSessionKey = 0;

        ((char*)&nPrivateKey)[nLeastSignificant] = szPrivateKey[nIndex * 2];
        ((char*)&nPrivateKey)[nMostSignificant] = szPrivateKey[(nIndex * 2) + 1];

        ((char*)&nPublicKey)[nLeastSignificant] = szPublicKey[nIndex * 2];
        ((char*)&nPublicKey)[nMostSignificant] = szPublicKey[(nIndex * 2) + 1];

        dh.setPrivateKey( nPrivateKey );
        nSessionKey = dh.getSessionKey( nPublicKey );

        szSessionKey[nIndex * 2] = ((char*)&nSessionKey)[nLeastSignificant];
        szSessionKey[(nIndex * 2) + 1] = ((char*)&nSessionKey)[nMostSignificant];
    }
}


/********************************************************************
********************************************************************/
int getMSBytePos()
{
    int nOne = 1;
    if ( ((char*)&nOne)[0] ) {
        return 1;
    } else {
        return 2;
    }
}


/********************************************************************
********************************************************************/
int getLSBytePos()
{
    int nOne = 1;
    if ( ((char*)&nOne)[0] ) {
        return 0;
    } else {
        return 3;
    }
}


/********************************************************************
    Assumption:  Passed data buffer is large enough to handle original
                 Message plus 16 more characters.
********************************************************************/
int encodeData( char* pData, int nSize, unsigned char* pSessionKey )
{
    init();
    if ( nSize >= 0 ) {
        int nIndex;
        int nIteration;
        int nRandom1 = rand() * rand();
        int nRandom2 = rand() * rand();

        //Make room for extra stuff
        memmove( pData + sizeof( szPacketCheck1 ) + sizeof(nRandom1), pData, nSize );
        
        //Add some salt to front
        memcpy( pData, &nRandom1, sizeof(nRandom1) );
        nSize += sizeof(nRandom1);

        //Add checksum to front
        memcpy( pData + sizeof(nRandom1), szPacketCheck1, sizeof( szPacketCheck1 ) );
        nSize += sizeof( szPacketCheck1 );

        //Add checksum to back
        memcpy( pData + nSize, szPacketCheck2, sizeof( szPacketCheck2 ) );
        nSize += sizeof( szPacketCheck2 );

        //Add some salt to back
        memcpy( pData + nSize, &nRandom2, sizeof(nRandom2) );
        nSize += sizeof(nRandom2);


        //Pass 1:  Ensure message bits are all dependent on each other
        //         One bit changed scramble the rest
        for ( nIteration = (nNumRounds - 1); nIteration >= 0; --nIteration ) {
            for ( nIndex = 0; nIndex < nSize; ++nIndex ) {
                pData[nIndex] ^= pData[(nIndex + 1 + nIteration) % nSize];

                pData[ (nIndex + 2) % nSize ] ^= pData[nIndex];
            }

            for ( nIndex = nSize - 1; nIndex >= 0; --nIndex ) {
                pData[nIndex] ^= pData[(nIndex + nSize - 1 - nIteration ) % nSize];

                pData[ (nIndex + nSize - 1 - 2 ) % nSize ] ^= pData[nIndex];
            }
        }

        //Pass 2:  Scramble with session key
        for ( nIndex = 0; nIndex < nSize; ++nIndex ) {
            pData[nIndex] = pData[nIndex] ^ pSessionKey[ nIndex % SESSION_KEY_SIZE ];
        }

        return nSize;
    } else {
        return -1;
    }
}


/********************************************************************
********************************************************************/
int decodeData( char* pData, int nSize, unsigned char* pSessionKey )
{
    if ( nSize >= 0 ) {
        int nIndex;
        int nIteration;

        // Undo Pass 2
        for ( nIndex = 0; nIndex < nSize; ++nIndex ) {
            pData[nIndex] = pData[nIndex] ^ pSessionKey[ nIndex % SESSION_KEY_SIZE ];
        }

        // Undo Pass 1
        for ( nIteration = 0; nIteration < nNumRounds; ++nIteration ) {
            for ( nIndex = 0; nIndex < nSize; ++nIndex ) {
                pData[ (nIndex + nSize - 1 - 2 ) % nSize ] ^= pData[nIndex];

                pData[nIndex] ^= pData[(nIndex + nSize - 1 - nIteration ) % nSize];
            }

            for ( nIndex = nSize - 1; nIndex >= 0; --nIndex ) {
                pData[ (nIndex + 2) % nSize ] ^= pData[nIndex];

                pData[nIndex] ^= pData[(nIndex + 1 + nIteration) % nSize];
            }
        }

        //Compare check array
        if ( memcmp( pData + sizeof(int), szPacketCheck1, sizeof( szPacketCheck1 ) ) != 0 
            || memcmp( pData + ( nSize - sizeof( szPacketCheck2 ) - sizeof(int) ), szPacketCheck2, sizeof( szPacketCheck2 ) ) != 0 ) {
            memset( pData, 0, nSize );
            return -1;
        }

        //Remove Check array
        memmove( pData, pData + sizeof( szPacketCheck1 ) + sizeof(int), nSize - sizeof( szPacketCheck1 ) - sizeof(int) );

        //Return original size
        return nSize - sizeof( szPacketCheck1 ) - sizeof( szPacketCheck2 ) - sizeof(int) - sizeof(int);
    } else {
        return -1;
    }
}


/********************************************************************
********************************************************************/
static void init()
{
    static int bInitialized = 0;
    if ( !bInitialized ) {
        bInitialized = 1;
        srand( time( 0 ) );
    }
}

