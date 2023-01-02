/*
MIT License

Copyright(c) 2023 Lucas Bigorenski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/



#ifndef __MALLOCLIB__
#define __MALLOCLIB__
#include <stdint.h> //definition for uint32_t and uint64_t
#include <stdbool.h> //bool type
#include <string.h> //memcpy and memset





#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))

#ifdef __x86_64__
//MWORD For x86 = uint32_t - For x86_64 = uint64_t
#define MWORD uint64_t
#define MM_DATA_PER_BLOCK	1018 //how many free uin32_t in a page (4kb - 3*MWORD)
#else 
//MWORD For x86 = uint32_t - For x86_64 = uint64_t
#define MWORD uint32_t //x86 so MWORD = uint64_t
#define MM_DATA_PER_BLOCK	1021 //how many free uin32_t in a page (4kb - 3*MWORD)
#endif

#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))

#ifdef _M_AMD64	
#define MWORD uint64_t
#define MM_DATA_PER_BLOCK	1018 //how many free uin32_t in a page (4kb - 3*MWORD)
#else 
#define MWORD uint32_t
#define MM_DATA_PER_BLOCK	1021 //how many free uin32_t in a page (4kb - 3*MWORD)
#endif

#endif





extern volatile bool MM_INITIALIZED;
typedef struct {
	bool used;
	MWORD size;
}header_t;

#define MM_PAGE_SIZE		4096

PACK(typedef struct {
	MWORD* next; //next data or page block
	MWORD free; //how much is left
	MWORD size; //how many sub-blocks
	uint32_t data[MM_DATA_PER_BLOCK]; //minimum size
}block_t;);


//Implementation needs to be provided for
void* mmGetPages(MWORD n);
void mmReleasePages(MWORD* address, MWORD n);


//Internal use
void* lmAllocInBlock(block_t *block, MWORD size);


//external use
MWORD lmInit();
void* lmMalloc(MWORD size);
MWORD lmFree(void *ptr);



//void mmListAllMem();
//void mmListPageContent(block_t* thisPage);
#endif

