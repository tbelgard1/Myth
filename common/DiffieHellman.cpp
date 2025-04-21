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
    DiffieHellman.cpp                                       //WCK025
********************************************************************/

#include "DiffieHellman.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include <assert.h>


int CDiffieHellman::m_bInitialized = false;


/********************************************************************
********************************************************************/
CDiffieHellman::CDiffieHellman( int nPrime, int nGenerator ) 
: m_nPrime( nPrime ), m_nGenerator( nGenerator ), m_nPrivateKey( -1 )
{
    assert( nPrime < 32768 );
    assert( nGenerator < nPrime );

    if ( !m_bInitialized ) {
        srand( (unsigned)time( NULL ) );
        m_bInitialized = true;
    }
}


/********************************************************************
********************************************************************/
int CDiffieHellman::getPrivateKey()
{
    if ( m_nPrivateKey < 0 ) {
        makePrivateKey();
    }
    return m_nPrivateKey;
}


/********************************************************************
********************************************************************/
int CDiffieHellman::getPublicKey()
{
    if ( m_nPrivateKey < 0 ) {
        makePrivateKey();
    }
    return powerMod( m_nGenerator, m_nPrivateKey, m_nPrime );
}


/********************************************************************
********************************************************************/
int CDiffieHellman::getSessionKey( int nPublicKey2 )
{
    if ( m_nPrivateKey < 0 ) {
        makePrivateKey();
    }
    return powerMod( nPublicKey2, m_nPrivateKey, m_nPrime );
}


/********************************************************************
********************************************************************/
int CDiffieHellman::powerMod( int nBase, int nExponent, int nModulus )
{
    int nResult = 1;
    for ( int nIndex = nExponent; nIndex > 0; --nIndex ) { 
        nResult *= nBase;
        nResult = nResult % nModulus;
    }
    return nResult;
}


/********************************************************************
********************************************************************/
void CDiffieHellman::makePrivateKey()
{
    m_nPrivateKey = rand() % ( m_nPrime - 1 );
}


/********************************************************************
********************************************************************/
void CDiffieHellman::setPrivateKey( int nPrivateKey )
{
    m_nPrivateKey = nPrivateKey;
}



