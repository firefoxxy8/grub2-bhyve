/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_EMU_BHYVE_H
#define GRUB_EMU_BHYVE_H 1

#include <grub/i386/relocator.h>

#ifndef __FreeBSD__
#error bhyve only applies for FreeBSD systems
#endif

#define DEFAULT_GUESTMEM	256

struct grub_bhyve_info {
  int bootsz;
  int nsegs;
  struct grub_mmap_region *segs;
};

int grub_emu_bhyve_init(const char *vmname, grub_uint64_t memsz);
void EXPORT_FUNC(grub_emu_bhyve_boot32)(grub_uint32_t bootaddr, 
		     struct grub_relocator32_state rs);
const struct grub_bhyve_info * EXPORT_FUNC(grub_emu_bhyve_info) (void);
void * EXPORT_FUNC(grub_emu_bhyve_virt) (grub_uint64_t physaddr);

#endif /* GRUB_EMU_BHYVE_H */
