/* This file is part of Into.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * IMPORTANT LICENSING INFORMATION
 *
 * Into is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Into is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "PiiSimpleMemoryManager.h"
#include <PiiBits.h>
#include <cstdlib>

/* Memory arrangement
 *
 *   +- head
 *   |
 *   v
 * +-+-------+-+-------+-+-------+-+ ... +-------+-+
 * | | block |p| block |p| block |p|     | block |p|
 * +-+-------+-+-------+-+-------+-+ ... +-------+-+
 *            |^        |^        |      ^
 *            ||        ||        |      |
 *            ++        ++        +------+
 *
 * Each memory block is followed by a void* that points to the
 * beginning of the next free memory block. Each memory block is
 * aligned at a 16-byte boundary. In the beginning, empty space may be
 * left for proper alignment. The head pointer always points to the
 * first free memory block.
 */

PiiSimpleMemoryManager::Data::Data(size_t memorySize, size_t blockSize) :
  // Align blocks to 16-byte boundaries
  blockSize(Pii::alignAddress(static_cast<size_t>(blockSize + sizeof(void*)), 0xf)),
  pMemory(malloc(memorySize)), // Allocate memory block from heap
  pHead(0) // Initialize the manager as fully allocated
{
}

PiiSimpleMemoryManager::PiiSimpleMemoryManager(size_t memorySize, size_t blockSize) :
  d(new Data(memorySize, blockSize))
{
  // Align first block
  size_t startAddress = reinterpret_cast<size_t>(Pii::alignAddress(d->pMemory, 0xf));
  // Bytes available after alignment
  size_t bytesAvailable = reinterpret_cast<size_t>(d->pMemory) + memorySize - startAddress;
  // Total number of blocks
  d->lBlockCount = long(bytesAvailable/d->blockSize);
  // Start of last block
  d->pLastAddress = reinterpret_cast<void*>(startAddress + (d->lBlockCount-1) * d->blockSize);

  size_t fullBlockSize = d->blockSize;
  // This is the number of bytes available to the user
  d->blockSize -= sizeof(void*);
  
  // Optimization...
  d->lLastAddress = (unsigned long)d->pLastAddress - (unsigned long)d->pMemory;

  // Since head is now zero, the buffer is full. We need to release
  // all memory blocks.
  char* pBuffer = static_cast<char*>(d->pLastAddress);
  for (long i=0; i<d->lBlockCount; ++i, pBuffer -= fullBlockSize)
    deallocate(pBuffer);
}

PiiSimpleMemoryManager::~PiiSimpleMemoryManager()
{
  free(d->pMemory);
  delete d;
}

#define PII_NEXT_POINTER(buffer) \
  reinterpret_cast<void**>(static_cast<char*>(buffer) + d->blockSize)

void* PiiSimpleMemoryManager::allocate(size_t bytes)
{
  // No need to allocate anything.
  if (bytes == 0)
    return 0;
  
  d->mutex.lock();
  // Do we have free blocks? Can we allocate this many bytes?
  if (d->pHead != 0 && bytes <= d->blockSize)
    {
      void* pBuffer = d->pHead;
      // Move the head pointer to the next free block
      d->pHead = *PII_NEXT_POINTER(d->pHead);
      d->mutex.unlock();
      return pBuffer;
    }
  d->mutex.unlock();
  return 0;
}

bool PiiSimpleMemoryManager::deallocate(void* buffer)
{
  if (buffer == 0)
    return true;
  
  d->mutex.lock();
  //if (buffer >= d->pMemory && buffer <= d->pLastAddress)
  if ((unsigned long)buffer - (unsigned long)d->pMemory <= d->lLastAddress)
    {
      // Make the pointer at the end of the released buffer to point
      // to the old head.
      *PII_NEXT_POINTER(buffer) = d->pHead;
      // Mark the just released buffer as the new head.
      d->pHead = buffer;
      d->mutex.unlock();
      return true;
    }
  d->mutex.unlock();
  return false;
}

long PiiSimpleMemoryManager::blockCount() const
{
  return d->lBlockCount;
}
