#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/*
PLATFORM.H
what is the target platform?

macintosh
windows
playstation
*/

#if defined(__MWERKS__) // metrowerks
	#if defined(__INTEL__) // intel
		#define intel
		#define windows
	#elif defined(__POWERPC__) // macintosh powerpc
		#define powerpc
		#define macintosh
	#else
		#error "unknown metrowerks target"
	#endif

#elif defined(_MSC_VER) // microsoft
	#define intel
	#define windows

#elif defined(__MRC__) // mpw mrc
	#define powerpc
	// macintosh is a built-in in MrC and cannot be redefined
	
#elif defined(__MOTO__) // mpw motorola
	#define powerpc
	#define macintosh

#elif defined(__GNUC__) // gnu c
        #ifdef psx // playstation
                #error "look before you leap"
        #elif defined(powerpc)
        	// MkLinux
        	// linux is already defined by the compiler
        #else
        	// standard linux
        	#define intel
        #endif

#elif defined(__WATCOMC__)  // Watcom C
	#define intel				// windoze build
	#define windows
	#define MICROSOFT 			// think that we are using microsoft.

#else
	#error "unknown compiler"
#endif

#ifdef powerpc
	#define big_endian
#elif defined(intel)
	#define little_endian
#else
#error "Unknown target architecture"
#endif

#ifdef windows
#elif defined(macintosh)
#elif defined(linux)
#else
#error "Unknown target operating system"
#endif

#endif // __PLATFORM_H__


