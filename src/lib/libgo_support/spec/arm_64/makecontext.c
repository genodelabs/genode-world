/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2003 Marcel Moolenaar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: releng/12.0/lib/libc/amd64/gen/makecontext.c 326193 2017-11-25 17:12:48Z pfg $");

#include <sys/types.h>
#include <sys/ucontext.h>
#include <stdarg.h>
#include <stdlib.h>

__weak_reference(__makecontext, makecontext);


/* makecontext sets up a stack and the registers for the
   user context.  The stack looks like this:

               +-----------------------+
               | padding as required   |
               +-----------------------+
    sp ->      | parameter 7-n         |
               +-----------------------+

   The registers are set up like this:
     %x0 .. %x7: parameter 1 to 8
     %x19   : uc_link
     %sp    : stack pointer.
*/

void
__makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
  extern void __startcontext (void);
  uint64_t *sp;
  va_list ap;
  int i;

  sp = (uint64_t *)
    ((uintptr_t) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);

  /* Allocate stack arguments.  */
  sp -= argc < 8 ? 0 : argc - 8;

  /* Keep the stack aligned.  */
  sp = (uint64_t *) (((uintptr_t) sp) & -16L);

  ucp->uc_mcontext.mc_gpregs.gp_x[19] = (uintptr_t)ucp->uc_link;
  ucp->uc_mcontext.mc_gpregs.gp_sp = (uintptr_t)sp;
  ucp->uc_mcontext.mc_gpregs.gp_elr = (uintptr_t)func; // oPC
  ucp->uc_mcontext.mc_gpregs.gp_x[29] = (uintptr_t)0;
  ucp->uc_mcontext.mc_gpregs.gp_lr = (uintptr_t)&__startcontext; // oLR AKA X30

  va_start (ap, argc);
  for (i = 0; i < argc; ++i)
    if (i < 8)
      ucp->uc_mcontext.mc_gpregs.gp_x[i] = va_arg (ap, uint64_t);
    else
      sp[i - 8] = va_arg (ap, uint64_t);

  va_end (ap);
}
