/*
  MiniVitaTV
  Copyright (C) 2018, TheFloW

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <psp2/kernel/modulemgr.h>
#include <taihen.h>

static SceUID hooks[2];

static tai_hook_ref_t scePafMiscIsVitaTVRef;
static tai_hook_ref_t scePafMiscIsMultiControllerSupportedRef;

static int scePafMiscIsVitaTVPatched() {
  TAI_CONTINUE(int, scePafMiscIsVitaTVRef);
  return 1;
}

static int scePafMiscIsMultiControllerSupportedPatched() {
  TAI_CONTINUE(int, scePafMiscIsMultiControllerSupportedRef);
  return 1;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
  hooks[0] = taiHookFunctionImport(&scePafMiscIsVitaTVRef, "SceShell", 0x3D643CE8,
                                   0xAF4FC3F4, scePafMiscIsVitaTVPatched);
  hooks[1] = taiHookFunctionImport(&scePafMiscIsMultiControllerSupportedRef, "SceShell", 0x3D643CE8,
                                   0x2C227FFD, scePafMiscIsMultiControllerSupportedPatched);

  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
  if (hooks[1] >= 0)
    taiHookRelease(hooks[1], scePafMiscIsMultiControllerSupportedRef);
  if (hooks[0] >= 0)
    taiHookRelease(hooks[0], scePafMiscIsVitaTVRef);

  return SCE_KERNEL_STOP_SUCCESS;
}
