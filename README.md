# NuCachis
A simple, interactive cache simulator


# Limitations
 - Memory is 32 bit (Word) addressable only.
 - WB caches are Write-Allocate only.
 - In main memory, access times apply equally to loads and stores (Writing 4 words takes as much as reading 4 words, 1 * first access time + 3 * burst access time)