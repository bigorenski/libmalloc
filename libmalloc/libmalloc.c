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


#include "libmalloc.h"

volatile bool MM_INITIALIZED = false;
block_t* firstblock;

void* mmGetPages(MWORD n) {

	//You have to provide a way to get n 4kb contiguous pages
	

}

void mmReleasePages(MWORD* address, MWORD n) {

	//You have to provide a way to release n contigouos pages starting from 'address'


}

MWORD lmInit(){
	
	firstblock = (block_t*)mmGetPages(1);
	if (!firstblock)return(-1); //Couldn't alloc first block/page

	memset(firstblock, 0, MM_PAGE_SIZE); //We rely on the block being filled with zeroes
	firstblock->next = NULL;
	firstblock->free = MM_DATA_PER_BLOCK * (sizeof(uint32_t));
	firstblock->size = MM_DATA_PER_BLOCK;

	MM_INITIALIZED = true;
	return(0);
}





void* lmMalloc(MWORD size) {

	if (!MM_INITIALIZED)return(NULL); //libmalloc wasn't initialized
	if (size == 0)return(NULL); //We were asked for 0 bytes
	

	block_t* thisBlock = firstblock; //Begin searching in the firstblock(allocated by mmInit)

	SEARCH_FOR_FREE_SPACE: //Do the search... If a new block needs to be allocated we'll jump back here with a goto after
	while (thisBlock != NULL) {
		if ((size + (sizeof(header_t))) < thisBlock->free) {
			return(lmAllocInBlock(thisBlock, size)); //There's enough memory in this block
		}

		thisBlock = (block_t*)thisBlock->next; //Not enough memory, search the next block
	}


	//We haven't found enough memory in any allocated block
	thisBlock = firstblock; //reset pointer
	while (thisBlock->next != NULL)thisBlock = (block_t*)thisBlock->next; //Find the spot to include a new block


	//For this 'size' we need just one more block, let's get it
	if ((size + (sizeof(header_t))) <= (MM_DATA_PER_BLOCK * (sizeof(MWORD)))) {
		thisBlock->next = mmGetPages(1);
		if (!thisBlock->next)return(NULL); //Couldn't get a new block

		thisBlock = (block_t*)thisBlock->next; //Put the block in the chain
		memset(thisBlock, 0, sizeof(block_t)); //We rely on blocks being zeroed

		thisBlock->next = NULL;
		thisBlock->free = MM_DATA_PER_BLOCK * sizeof(uint32_t);
		thisBlock->size = MM_DATA_PER_BLOCK;
		goto SEARCH_FOR_FREE_SPACE; //We got a new block in the chain now, go alloc
	}


	//For this 'size' we need more than one 4kb block, let's get a bigger block
	if ((size + (sizeof(header_t))) > (MM_DATA_PER_BLOCK * (sizeof(MWORD)))) {
		MWORD pages = (size + (sizeof(header_t))) / MM_PAGE_SIZE + 1; //Calc new block size(multiples of 4kb)
		thisBlock->next = mmGetPages(pages);
		if (!thisBlock->next)return(NULL); //Couldn't get a new block of that size

		thisBlock = (block_t*)thisBlock->next; //Put new block in the chain
		memset(thisBlock, 0, MM_PAGE_SIZE * pages); //We rely on the block being zeroed
		thisBlock->next = NULL;

		thisBlock->free = pages * (MM_DATA_PER_BLOCK * (sizeof(MWORD))); //This new block is bigger than 4kb
		thisBlock->size = MM_DATA_PER_BLOCK * pages; //This new block is bigger than 4kb
		goto SEARCH_FOR_FREE_SPACE; //We got a new block in the chain now, go alloc
	}

	return(NULL); //We should not reach here


}

void* lmAllocInBlock(block_t *block, MWORD size) {
	if (!block) return(NULL); //We got a null block
	if (block->free < (size + sizeof(header_t))) return(NULL); //We got a block that is smaller than what we need
	
	header_t* header = (header_t*)& block->data[0]; //Point header to start of the data section on the block
	MWORD k = block->size;

	while ((MWORD)header < (MWORD) & block->data[k - 1]) { //Until we reach end of data section, search for spot

		if (header->used == false && (header->size == 0 || header->size <= size)) //Found a spot with zero size(new block), or <= required size
			goto ALLOC_NOW; //Go alloc

		header = (header_t*)(((MWORD)header) + header->size + sizeof(header_t)); //Haven't found, jump to next header

	}
	
	//Couldn't find a spot big enough
	return(NULL);

	ALLOC_NOW: //Everything set, alloc now
	if (header->size > size) {
		//This spot is bigger than what we need, let's split it
		header_t *newHeader = (header_t*)(((MWORD)header) + sizeof(header_t) + size);
		newHeader->used = false;
		newHeader->size = header->size - size;
	}


	//Fill metadata on the new block
	header->used = true;
	header->size = size;
	block->free -= (size + sizeof(header_t));
	
	//Return pointer to data spot (header + sizeof(header)
	return(header + 1);

}

MWORD lmFree(void* ptr) {
	if (!ptr)return(0); //Received a NULL pointer

	header_t *header = (header_t*)((MWORD)ptr - sizeof(header_t)); //Header is sizeof(header_t) behind pointer
	

	block_t *page = firstblock; //Start the search by the first page
	MWORD k = firstblock->size;

	while (page != NULL) { //Search in pages until a NULL page is found(next == NULL)
		
		if (((MWORD)header > (MWORD)page) && ((MWORD)header < (MWORD)&page->data[k - 1]))goto FOUND; //Found our header
		page = (block_t*)page->next;

	}
	
	//We couldn't find this pointer in any page
	return(0); //Return 0
	
	FOUND: //Jumped here from the while loop(found our pointer)
	header->used = false;
	MWORD originalSize = header->size;
	memset(header + 1, 0, header->size); //is this necessary?
	

	header_t *newHeader = (header_t*)(((MWORD)header) + header->size); //Take a look at the next header, if it is empty we should join the two headers

	while (((MWORD)newHeader < (MWORD)page->data[MM_DATA_PER_BLOCK]) && (newHeader->used == false)) {


		if (newHeader->used == false) {

			header->size += newHeader->size;

		}
		newHeader = (header_t*)(((MWORD)newHeader) + newHeader->size);
	}


	page->free += header->size +(sizeof(header_t)); //Give space back to the page

	if (page->free >= ((page->size) * (sizeof(MWORD)) - sizeof(header))) { //If page is all free
		
		if (page == firstblock){ //Page is 'firstblock', so we won't give it back to the system, but we need to zero it
			
			block_t* next = (block_t*)page->next;
			
			memset(page, 0, MM_PAGE_SIZE); 
			page->free = MM_DATA_PER_BLOCK * sizeof(MWORD);
			page->size = MM_DATA_PER_BLOCK;
			page->next = (MWORD*)next;

			header_t *tmpHeader = (header_t*) & page->data[0];
			tmpHeader->used = false;
			tmpHeader->size = 0;
			return(originalSize);  //Return how much data we freed(pointer data)
		}

		//This is now the first page, so let's give it back to the system
		block_t* prevPage = firstblock;
		while (prevPage->next != (MWORD*)page)prevPage = (block_t*)prevPage->next;

		if (prevPage == NULL) return(0);

		prevPage->next = page->next;
		//printf("Freeing page at 0x%x\n", page);

		MWORD howManyPages = page->size / MM_PAGE_SIZE + 1;

		mmReleasePages((MWORD*)page, howManyPages);
	}

	page = firstblock;


	//Clean pages by searching for pages with no data (excluding 'firstBlock')
	while (page != NULL) {

		if (page->free >= (page->size * sizeof(MWORD)))
			if (page != firstblock) {
				MWORD howManyPages = page->size / MM_PAGE_SIZE + 1;

				mmReleasePages((MWORD*)page, howManyPages);
			}
		page = (block_t*)page->next;

	}
	
	return(originalSize); //Return how much data was freed(from pointer, not cleaned pages)
}
