# libmalloc
<b>Simple library providing dynamic memory allocation (similar to malloc) for hobby OSes and embedded systems</b>

<b>This library dependes on:</b><br>
<b>void* mmGetPages(MWORD n)</b> - Returns a pointer to n contiguous pages (4kb each) - This should implement some kind of lock/unlock<br>
<b>void mmReleasePages(MWORD* address, MWORD n)</b> - returns nothing. Release pages to the system - This should implement some kind of lock/unlock<br>
<b><stdint.h></b> - Definitions for uint32_t and uint64_t(for x86_64 only)<br>
<b><stdbool.h></b> - Definition for bool type<br>
<b><string.h></b> - memcpy and memset<br>

<b>---------------------------------------------------------------------------------------------</b><br>

<b>This library provides:</b><br>
<b>MWORD</b> - Macro. Extends to uint32_t or uint64_t depending on target<br>
<b>MWORD lmInit()</b> - Initialize the library. Returns 0 on success<br>
<b>void* lmMalloc(MWORD size)</b> - Similar to malloc. Returns pointer to allocated space<br>
<b>MWORD lmFree(void *ptr)</b> - Returns how much data was freed<br>



