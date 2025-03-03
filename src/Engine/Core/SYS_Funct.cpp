/*
 *  SYS_Funct.mm
 *  MobiTracker
 *
 *  Created by Marcin Skoczylas on 10-07-15.
 *  Copyright 2010 Marcin Skoczylas. All rights reserved.
 *
 */

#include "SYS_Funct.h"
#include "SYS_Main.h"
#include "SYS_FileSystem.h"
#include "CSlrString.h"
#include "json.hpp"

#if !defined(WIN32) && !defined(ANDROID)
#include <unistd.h>
#include <execinfo.h>
#include <sys/param.h>
#include <sys/times.h>
#include <sys/types.h>
#endif

#ifdef WIN32
#pragma comment(lib, "psapi.lib")
#include <psapi.h>
#endif

#if !defined(WIN32) && !defined(ANDROID)
#include <fstream>
#endif

#include <string.h>

#if defined(LINUX) || defined(MACOS)
#include <pthread.h>
#include <unistd.h>
#endif

// http://www.rawmaterialsoftware.com/viewtopic.php?f=4&t=4804

// TODO: -ffast-math

// TODO: hashtable: http://cboard.cprogramming.com/c-programming/152990-defn-inside-hash-table.html

unsigned NextPow2( unsigned x )
{
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
}

bool compare_str_num(char *str1, char *str2, u16 numChars)
{
	for (u16 i = 0; i != numChars; i++)
	{
		if (str1[i] != str2[i])
			return false;
	}
	return true;
}

void SYS_FixFileNameSlashes(char *buf)
{
	int len = strlen(buf);

	// normalize path separators
	for (int i = 0; i < len; i++)
	{
		if (buf[i] == '\\' || buf[i] == '/')
			buf[i] = SYS_FILE_SYSTEM_PATH_SEPARATOR;
	}
	
	// check if double path separators exist (fix for c:\dupa\/file.txt paths)
	for (int i = 0; i < len-1; i++)
	{
		if (buf[i] == SYS_FILE_SYSTEM_PATH_SEPARATOR
			&& buf[i+1] == SYS_FILE_SYSTEM_PATH_SEPARATOR)
		{
			for (int j = i; j < len-1; j++)
			{
				buf[j] = buf[j+1];
			}
			buf[len-1] = '\0';
			len--;
		}
	}
}

bool SYS_FileNameHasExtension(char *fileName, char *extension)
{
	int i = strlen(fileName) - 1;
	int j = strlen(extension) - 1;

	// x.ext  - min 2 more
	if (i < (j+2))
		return false;

	for ( ; j >= 0; )
	{
		if (fileName[i] != extension[j])
			return false;

		i--;
		j--;
	}

	if (fileName[i] != '.')
		return false;

	return true;
}

void SYS_RemoveFileNameExtension(char *fileName)
{
	// warning! if the fileName is const it will crash...
	// don't forget to not use extensions in the const char* filenames

	int l = strlen(fileName);

	bool isExt = false;
	for (int z = 0; z < l; z++)
	{
		if (fileName[z] == '.')
		{
			isExt = true;
			break;
		}
	}
	if (isExt == false)
		return;

	int pos = l;

	int i = l-1;

	for ( ; i >= 0; )
	{
		if (fileName[i] == '.')
		{
			pos = i;
			break;
		}

		i--;
	}

	if (pos != l)
	{
		fileName[pos] = 0x00;
	}
}

void SYS_Sleep(unsigned long milliseconds)
{
	//LOGD("SYS_Sleep %d", milliseconds);
	
#ifdef WIN32
	Sleep(milliseconds);
#else
	
	long milisec = milliseconds;
	
	struct timespec req={0};
	time_t sec=(int)(milisec/1000);
	milisec = milisec-(sec*1000);
	req.tv_sec=sec;
	req.tv_nsec=milisec*1000000L;
	while(nanosleep(&req,&req)==-1)
		continue;
	
#endif
	
	//LOGD("SYS_Sleep of %d done", milliseconds);
}

bool FUN_IsNumber(char c)
{
	if (c >= '0' && c <= '9')
		return true;
	
	return false;
}

bool FUN_IsNumber(char *str)
{
	u64 l = strlen(str);
	for (u64 n = 0; n < l; n++)
	{
		if (!isdigit( str[ n ] ))
		{
			return false;
		}
	}
	return true;
}

bool FUN_IsHexDigit(char c)
{
	if (c >= '0' && c <= '9')
		return true;

	if (c >= 'a' && c <= 'f')
		return true;

	if (c >= 'A' && c <= 'F')
		return true;
	
	return false;
}

bool FUN_IsHexNumber(char *str)
{
	u64 l = strlen(str);
	for (u64 i = 0; i < l; i++)
	{
		if (!FUN_IsHexDigit(str[i]))
		{
			return false;
		}
	}
	
	return true;
}

char FUN_HexDigitToChar(u8 hexDigit)
{
	u8 h = hexDigit % 16;
	char hexDigits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	return hexDigits[h];
}

u32 FUN_HexStrToValue(char *str)
{
	u32 value;
	sscanf(str, "%x", &value);
	return value;
}

u64 FUN_DecOrHexStrWithPrefixToU64(const char *str)
{
	// If the string starts with "0x" or "0X", interpret as hexadecimal
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
		return strtoul(str, NULL, 16);
	}

	// If the string starts with "$", interpret as hexadecimal (after the $)
	if (str[0] == '$') {
		return strtoul(str + 1, NULL, 16);
	}

	// Otherwise, interpret as a normal decimal integer
	return strtoul(str, NULL, 10);
}

u64 FUN_JsonValueDecOrHexStrWithPrefixToU64(const nlohmann::json& value)
{
	if (value.is_string())
	{
		return FUN_DecOrHexStrWithPrefixToU64(value.get<string>().c_str());
	}
	return value.get<u64>();
}

void FUN_IntToBinaryStr(unsigned value, char* binaryStr, int n)
{
	for (int i = 0; i < n; i++)
	{
		binaryStr[i] = (value & (int)1<<(n-i-1)) ? '1' : '0';
	}
	binaryStr[n]='\0';
}

void FUN_ToUpperCaseStr(char *str)
{
	u64 l = strlen(str);
	for (u64 i = 0; i < l; i++)
	{
		str[i] = toupper(str[i]);
	}
}

#if !defined(WIN32) && !defined(ANDROID) && !defined(IPHONE)

void process_mem_usage(double& vm_usage, double& resident_set)
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}
#endif

void SYS_PrintMemoryUsed()
{
#ifdef WIN32
	PPROCESS_MEMORY_COUNTERS pMemCountr = new PROCESS_MEMORY_COUNTERS;
	if( GetProcessMemoryInfo(GetCurrentProcess(), pMemCountr, sizeof(PROCESS_MEMORY_COUNTERS)))
	{
		LOGM("MEMORY=%d", pMemCountr->WorkingSetSize);
	}
	delete pMemCountr;
#elif !defined(ANDROID) && !defined(IPHONE)
	double vm, rss;
	process_mem_usage(vm, rss);

	LOGMEM("MEMORY VM=%d RSS=%d", (int)vm, (int)rss);
#else
	LOGError("SYS_PrintMemoryUsed: not implemented");
#endif
}

char *SYS_GetFileNameWithExtensionFromFullPath(const char *fileNameFull)
{
	int len = strlen(fileNameFull);
	
	if (len == 0)
	{
		char *ret = new char[1];
		ret[0] = 0x00;
		return ret;
	}
	
	// check if there's a path sign inside
	bool foundPathSign = false;
	for (int i = 0; i < len; i++)
	{
		if (fileNameFull[i] == '/' || fileNameFull[i] == '\\')
		{
			foundPathSign = true;
			break;
		}
	}
	
	// path sign is not found, just dup the original path as filename
	if (foundPathSign == false)
	{
		char *ret = new char[len+1];
		strcpy(ret, fileNameFull);
		return ret;
	}
	
	char *fileName = SYS_GetCharBuf();
	int i = len-1;

	for ( ; i >= 0; i--)
	{
		if (fileNameFull[i] == '/' || fileNameFull[i] == '\\')
		{
			i++;
			break;
		}
	}

	u32 j = 0;
	for ( ; i < len; i++, j++)
	{
		fileName[j] = fileNameFull[i];
	}
	fileName[j] = 0x00;

	char *ret = new char[len];
	strcpy(ret, fileName);

	SYS_ReleaseCharBuf(fileName);
	
	return ret;
}

char *SYS_GetPathFromFullPath(const char *fileNameFull)
{
	char pathName[1024];
	u32 len = strlen(fileNameFull);
	int i = len-1;

	for ( ; i >= 0; i--)
	{
		if (fileNameFull[i] == '/' || fileNameFull[i] == '\\')
		{
			i++;
			break;
		}
	}

	u32 j = 0;
	for ( ; j < 1020; j++)
	{
		if (j == i)
			break;
		pathName[j] = fileNameFull[j];
	}
	pathName[j] = 0x00;

	return STRALLOC(pathName);

}

char *FUN_SafeConvertStdStringToCharArray(std::string inString)
{
	//	LOGD("FUN_SafeConvertStdStringToCharArray");
	int len = inString.length();
	
	//	LOGD("   len=%d", len);
	char *buf = new char[len+1];
	
	for (int i = 0; i < len; i++)
	{
		char c = inString[i];
		//		LOGD("    [%d]=%x '%c'", i, c, c);
		buf[i] = c;
	}
	
	buf[len] = 0x00;
	
	return buf;
}

const char hexTableSmall[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
const char hexTableSmallNoZero[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

void sprintfHexCode4(char *pszBuffer, uint8 value)
{
	pszBuffer[0] = hexTableSmall[(value) & 0x0F];
	pszBuffer[1] = 0x00;
}

void sprintfHexCode8(char *pszBuffer, uint8 value)
{
	pszBuffer[0] = hexTableSmall[(value >> 4) & 0x0F];
	pszBuffer[1] = hexTableSmall[(value) & 0x0F];
	pszBuffer[2] = 0x00;
}

void sprintfHexCode16(char *pszBuffer, uint16 value)
{
	pszBuffer[0] = hexTableSmall[(value >> 12) & 0x0F];
	pszBuffer[1] = hexTableSmall[(value >> 8) & 0x0F];
	pszBuffer[2] = hexTableSmall[(value >> 4) & 0x0F];
	pszBuffer[3] = hexTableSmall[(value) & 0x0F];
	pszBuffer[4] = 0x00;
}

void sprintfHexCode4WithoutZeroEnding(char *pszBuffer, uint8 value)
{
	pszBuffer[0] = hexTableSmall[(value) & 0x0F];
}

void sprintfHexCode8WithoutZeroEnding(char *pszBuffer, uint8 value)
{
	pszBuffer[0] = hexTableSmall[(value >> 4) & 0x0F];
	pszBuffer[1] = hexTableSmall[(value) & 0x0F];
}

void sprintfHexCode12WithoutZeroEnding(char *pszBuffer, uint16 value)
{
	pszBuffer[0] = hexTableSmall[(value >> 8) & 0x0F];
	pszBuffer[1] = hexTableSmall[(value >> 4) & 0x0F];
	pszBuffer[2] = hexTableSmall[(value) & 0x0F];
}

void sprintfHexCode16WithoutZeroEnding(char *pszBuffer, uint16 value)
{
	pszBuffer[0] = hexTableSmall[(value >> 12) & 0x0F];
	pszBuffer[1] = hexTableSmall[(value >> 8) & 0x0F];
	pszBuffer[2] = hexTableSmall[(value >> 4) & 0x0F];
	pszBuffer[3] = hexTableSmall[(value) & 0x0F];
}

void sprintfHexCode16WithoutZeroEndingAndNoLeadingZeros(char *pszBuffer, uint16 value)
{
	pszBuffer[0] = hexTableSmallNoZero[(value >> 12) & 0x0F];
	pszBuffer[1] = hexTableSmallNoZero[(value >> 8) & 0x0F];
	pszBuffer[2] = hexTableSmallNoZero[(value >> 4) & 0x0F];
	pszBuffer[3] = hexTableSmallNoZero[(value) & 0x0F];
	
	if (pszBuffer[0] == '0')
	{
		pszBuffer[0] = ' ';
		if (pszBuffer[1] == '0')
		{
			pszBuffer[1] = ' ';
			if (pszBuffer[2] == '0')
			{
				pszBuffer[2] = ' ';
			}
		}
	}
}


// special printf for numbers only
// see formatting information below
//  Print the number "n" in the given "base"
//  using exactly "numDigits"
//  print +/- if signed flag "isSigned" is TRUE
//  use the character specified in "padchar" to pad extra characters
//
//  Examples:
//  sprintfNum(pszBuffer, 6, 10, 6,  TRUE, ' ',   1234);  -->  " +1234"
//  sprintfNum(pszBuffer, 6, 10, 6, FALSE, '0',   1234);  -->  "001234"
//  sprintfNum(pszBuffer, 6, 16, 6, FALSE, '.', 0x5AA5);  -->  "..5AA5"
#define hexchar(x)  ((((x)&0x0F)>9)?((x)+'A'-10):((x)+'0'))
void sprintfNum(char *pszBuffer, int size, char base, char numDigits, char isSigned, char padchar, i64 n)
{
    char *ptr = pszBuffer;
	
    if (!pszBuffer)
    {
        return;
    }
	
    char *p, buf[32];
    unsigned long long x;
    unsigned char count;
	
    // prepare negative number
    if( isSigned && (n < 0) )
    {
        x = -n;
    }
    else
    {
        x = n;
    }
	
    // setup little string buffer
    count = (numDigits-1)-(isSigned?1:0);
    p = buf + sizeof (buf);
    *--p = '\0';
	
    // force calculation of first digit
    // (to prevent zero from not printing at all!!!)
    *--p = (char)hexchar(x%base);
    x = x / base;
    // calculate remaining digits
    while(count--)
    {
        if(x != 0)
        {
            // calculate next digit
            *--p = (char)hexchar(x%base);
            x /= base;
        }
        else
        {
            // no more digits left, pad out to desired length
            *--p = padchar;
        }
    }
	
    // apply signed notation if requested
    if( isSigned )
    {
        if(n < 0)
        {
            *--p = '-';
        }
        else if(n > 0)
        {
            *--p = '+';
        }
        else
        {
            *--p = ' ';
        }
    }
	
    // print the string right-justified
    count = numDigits;
    while(count--)
    {
        *ptr++ = *p++;
    }
	
    return;
}

void sprintfHexCode64(char *pszBuffer, u64 n)
{
	sprintfUnsignedNum(pszBuffer, 16, 16, 16, '0', n);
}

// special printf for numbers only
// see formatting information below
//  Print the number "n" in the given "base"
//  using exactly "numDigits"
//  print +/- if signed flag "isSigned" is TRUE
//  use the character specified in "padchar" to pad extra characters
//
//  Examples:
//  sprintfNum(pszBuffer, 6, 10, 6,  TRUE, ' ',   1234);  -->  " +1234"
//  sprintfNum(pszBuffer, 6, 10, 6, FALSE, '0',   1234);  -->  "001234"
//  sprintfNum(pszBuffer, 6, 16, 6, FALSE, '.', 0x5AA5);  -->  "..5AA5"
void sprintfUnsignedNum(char *pszBuffer, int size, char base, char numDigits, char padchar, u64 n)
{
    char *ptr = pszBuffer;
	
    if (!pszBuffer)
    {
        return;
    }
	
    char *p, buf[64];
    unsigned long long x;
    unsigned char count;
	
    {
        x = n;
    }
	
    // setup little string buffer
    count = (numDigits-1);
    p = buf + sizeof (buf);
    *--p = '\0';
	
    // force calculation of first digit
    // (to prevent zero from not printing at all!!!)
    *--p = (char)hexchar(x%base);
    x = x / base;
    // calculate remaining digits
    while(count--)
    {
        if(x != 0)
        {
            // calculate next digit
            *--p = (char)hexchar(x%base);
            x /= base;
        }
        else
        {
            // no more digits left, pad out to desired length
            *--p = padchar;
        }
    }
	
    // print the string right-justified
    count = numDigits;
    while(count--)
    {
        *ptr++ = *p++;
    }
	
	pszBuffer[numDigits] = 0x00;
	
    return;
}

// fuzzy search
int FUN_FuzzyScore( const char *str1, const char *str2 )
{
	int score = 0, consecutive = 0, maxerrors = 0;
	int charNum = 0;
	while( *str1 && *str2 )
	{
		int is_leading = (*str1 & 64) && !(str1[1] & 64);
		if( (*str1 & ~32) == (*str2 & ~32) )
		{
			int had_separator = 0;
			if (charNum != 0)
			{
				had_separator = (str1[-1] <= 32);
			}
			
			int x = had_separator || is_leading ? 10 : consecutive * 5;
			consecutive = 1;
			score += x;
			++str2;
		}
		else
		{
			int x = -1, y = is_leading * -3;
			consecutive = 0;
			score += x;
			maxerrors += y;
		}
		++str1;
		++charNum;
	}
	return score + (maxerrors < -9 ? -9 : maxerrors);
}

int FUN_FuzzySearch( const char *str, int num, const char *words[] )
{
	int scoremax = 0;
	int best = -1;
	for( int i = 0; i < num; ++i ) {
		int score = FUN_FuzzyScore( words[i], str );
		int record = ( score >= scoremax );
		int draw = ( score == scoremax );
		if( record ) {
			scoremax = score;
			if( !draw ) best = i;
			else best = best >= 0 && strlen(words[best]) < strlen(words[i]) ? best : i;
		}
	}
	return best;
}

// convert 8 numbers (bits 0 or 1) to 8-bit value
u8 FUN_BitsToByte(u8 bits[8])
{
	u8 result = 0;
	for (int i = 0; i < 8; i++) 
	{
		result |= (bits[i] & 1) << (7 - i);  // Shift and OR the bits into the result
	}
	return result;
}

/*
// return whether a number is a power of two
inline UInt32 IsPowerOfTwo(UInt32 x)
{
	return (x & (x-1)) == 0;
}

// count the leading zeroes in a word
#ifdef __MWERKS__

// Metrowerks Codewarrior. powerpc native count leading zeroes instruction:
#define CountLeadingZeroes(x)  ((int)__cntlzw((unsigned int)x))

#elif TARGET_OS_WIN32

static int CountLeadingZeroes( int arg )
{
	__asm{
		bsr eax, arg
		mov ecx, 63
		cmovz eax, ecx
		xor eax, 31
    }
    return arg;
}

#else

static __inline__ int CountLeadingZeroes(int arg) {
#if TARGET_CPU_PPC || TARGET_CPU_PPC64
	__asm__ volatile("cntlzw %0, %1" : "=r" (arg) : "r" (arg));
	return arg;
#elif TARGET_CPU_X86 || TARGET_CPU_X86_64
	__asm__ volatile(
					 "bsrl %0, %0\n\t"
					 "movl $63, %%ecx\n\t"
					 "cmove %%ecx, %0\n\t"
					 "xorl $31, %0"
					 : "=r" (arg)
					 : "0" (arg) : "%ecx"
					 );
	return arg;
#else
	if (arg == 0) return 32;
	return __builtin_clz(arg);
#endif
}

#endif

// count trailing zeroes
inline UInt32 CountTrailingZeroes(UInt32 x)
{
	return 32 - CountLeadingZeroes(~x & (x-1));
}

// count leading ones
inline UInt32 CountLeadingOnes(UInt32 x)
{
	return CountLeadingZeroes(~x);
}

// count trailing ones
inline UInt32 CountTrailingOnes(UInt32 x)
{
	return 32 - CountLeadingZeroes(x & (~x-1));
}

// number of bits required to represent x.
inline UInt32 NumBits(UInt32 x)
{
	return 32 - CountLeadingZeroes(x);
}

// base 2 log of next power of two greater or equal to x
inline UInt32 Log2Ceil(UInt32 x)
{
	return 32 - CountLeadingZeroes(x - 1);
}

// next power of two greater or equal to x
inline UInt32 NextPowerOfTwo(UInt32 x)
{
	return 1L << Log2Ceil(x);
}

// counting the one bits in a word
inline UInt32 CountOnes(UInt32 x)
{
	// secret magic algorithm for counting bits in a word.
	UInt32 t;
	x = x - ((x >> 1) & 0x55555555);
	t = ((x >> 2) & 0x33333333);
	x = (x & 0x33333333) + t;
	x = (x + (x >> 4)) & 0x0F0F0F0F;
	x = x + (x << 8);
	x = x + (x << 16);
	return x >> 24;
}

// counting the zero bits in a word
inline UInt32 CountZeroes(UInt32 x)
{
	return CountOnes(~x);
}

// return the bit position (0..31) of the least significant bit
inline UInt32 LSBitPos(UInt32 x)
{
	return CountTrailingZeroes(x & -(SInt32)x);
}

// isolate the least significant bit
inline UInt32 LSBit(UInt32 x)
{
	return x & -(SInt32)x;
}

// return the bit position (0..31) of the most significant bit
inline UInt32 MSBitPos(UInt32 x)
{
	return 31 - CountLeadingZeroes(x);
}

// isolate the most significant bit
inline UInt32 MSBit(UInt32 x)
{
	return 1UL << MSBitPos(x);
}

// Division optimized for power of 2 denominators
inline UInt32 DivInt(UInt32 numerator, UInt32 denominator)
{
	if(IsPowerOfTwo(denominator))
		return numerator >> (31 - CountLeadingZeroes(denominator));
	else
		return numerator/denominator;
}

*/
