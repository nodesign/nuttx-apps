/****************************************************************************
 * apps/graphics/tiff/tiff_utils.c
 *
 *   Copyright (C) 2011 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <spudmonkey@racsa.co.cr>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <apps/tiff.h>

#include "tiff_internal.h"

/****************************************************************************
 * Pre-Processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: tiff_put16 and tiff_put32
 *
 * Description:
 *   Put 16 and 32 values in the correct byte order at the specified position.
 *
 * Input Parameters:
 *   dest - The location to store the multi-byte data
 *   value - The value to be stored
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void tiff_put16(FAR uint8_t *dest, uint16_t value)
{
#ifdef CONFIG_ENDIAN_BIG
  *dest++ = (uint8_t)(value >> 8);
  *dest   = (uint8_t)(value & 0xff);
#else
  *dest++ = (uint8_t)(value & 0xff);
  *dest   = (uint8_t)(value >> 8);
#endif
}

void tiff_put32(FAR uint8_t *dest, uint32_t value)
{
#ifdef CONFIG_ENDIAN_BIG
  tiff_put16(dest,   (uint16_t)(value >> 16));
  tiff_put16(dest+2, (uint16_t)(value & 0xffff));
#else
  tiff_put16(dest,   (uint16_t)(value & 0xffff));
  tiff_put16(dest+2, (uint16_t)(value >> 16));
#endif
}

/****************************************************************************
 * Name: tiff_write
 *
 * Description:
 *   Write TIFF data to the specified file
 *
 * Input Parameters:
 *   fd - Open file descriptor to write to
 *   buffer - Read-only buffer containing the data to be written
 *   count - The number of bytes to write
 *
 * Returned Value:
 *   Zero (OK) on success.  A negated errno value on failure.
 *
 ****************************************************************************/

int tiff_write(int fd, FAR void *buffer, size_t count)
{
  ssize_t nbytes;
  int errval;

  /* This loop retries the write until either: (1) it completes successfully,
   * or (2) until an irrecoverble error occurs.
   */

  while (count > 0)
    {
      /* Do the write */

      nbytes = write(fd, buffer, count);

      /* Check for an error */

      if (nbytes < 0)
        {
          /* EINTR is not an error.. this just means that the write was
           * interrupted by a signal.
           */

          errval = errno;
          if (errval != EINTR)
            {
              /* Other errors are bad news and we will break out with an error */

              return -errval;
            }
        }

      /* What if the the write returns some number of bytes other than the requested number? */

      else
        {
          DEBUGASSERT(nbytes < count && nbytes != 0);
          count -= nbytes;
        }
    }

  return OK;
}
