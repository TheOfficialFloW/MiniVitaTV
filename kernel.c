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

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/processmgr.h>

#include <stdio.h>
#include <string.h>

#include <taihen.h>

#define MAX_CONTROLLERS 5

typedef struct {
  int unk_00;
  int unk_04;
  SceUID ctrl_eventid;
  SceUID ctrlext_eventid;
} ctrl_events_t;

int ksceKernelSysrootGetShellPid(void);
int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);
int module_get_offset(SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr);

static int (* _ksceKernelGetModuleInfo)(SceUID pid, SceUID modid, SceKernelModuleInfo *info) = NULL;

static tai_hook_ref_t ksceSysrootCheckModelCapabilityRef;

static SceUID hooks[2];

static int ksceSysrootCheckModelCapabilityPatched(int capability) {
  int res = TAI_CONTINUE(int, ksceSysrootCheckModelCapabilityRef, capability);

  // Support multi-controllers
  if (capability == 1) {
    return 1;
  }

  return res;
}

static int patch_bt() {
  int res;

  tai_module_info_t tai_info;
  tai_info.size = sizeof(tai_module_info_t);
  res = taiGetModuleInfoForKernel(KERNEL_PID, "SceBt", &tai_info);
  if (res < 0)
    return res;

  SceKernelModuleInfo mod_info;
  mod_info.size = sizeof(SceKernelModuleInfo);
  res = _ksceKernelGetModuleInfo(KERNEL_PID, tai_info.modid, &mod_info);
  if (res < 0)
    return res;

  uint32_t data_addr = (uint32_t)mod_info.segments[1].vaddr;

  // Add PSTV flags to support bluetooth controllers
  *(uint32_t *)(data_addr + 0x18) |= 0x10000;

  // Ignore branch to support DS3
  uint16_t nop_opcode = 0xBF00;
  hooks[0] = taiInjectDataForKernel(KERNEL_PID, tai_info.modid, 0, 0xD820, &nop_opcode, sizeof(uint16_t));

  return 0;
}

static int patch_ctrl(SceUID shell_pid) {
  int res;

  tai_module_info_t tai_info;
  tai_info.size = sizeof(tai_module_info_t);
  res = taiGetModuleInfoForKernel(KERNEL_PID, "SceCtrl", &tai_info);
  if (res < 0)
    return res;

  SceKernelModuleInfo mod_info;
  mod_info.size = sizeof(SceKernelModuleInfo);
  res = _ksceKernelGetModuleInfo(KERNEL_PID, tai_info.modid, &mod_info);
  if (res < 0)
    return res;

  uint32_t data_addr = (uint32_t)mod_info.segments[1].vaddr;

  // Free per-process ctrl heaps
  int (* free_process_heap)(SceUID pid);
  module_get_offset(KERNEL_PID, tai_info.modid, 0, 0xDF5, (uintptr_t *)&free_process_heap);
  free_process_heap(KERNEL_PID);
  if (shell_pid != -1)
    free_process_heap(shell_pid);

  // Hook function to support multi controllers
  hooks[1] = taiHookFunctionExportForKernel(KERNEL_PID, &ksceSysrootCheckModelCapabilityRef, "SceSysmem",
                                            TAI_ANY_LIBRARY, 0x8AA268D6, ksceSysrootCheckModelCapabilityPatched);

  // TODO: delete previous local storage
  *(uint32_t *)(data_addr + 0xA70) = ksceKernelCreateProcessLocalStorage("SceCtrl", 0x58);

  // Create bigger heap
  ksceKernelDeleteHeap(*(uint32_t *)(data_addr + 0xA6C));
  *(uint32_t *)(data_addr + 0xA6C) = ksceKernelCreateHeap("SceCtrlHeap", MAX_CONTROLLERS * 0xA100, NULL);

  // Create new events
  ctrl_events_t *events = (ctrl_events_t *)(data_addr + 0x14);

  int i;
  for (i = 0; i < MAX_CONTROLLERS; i++) {
    ksceKernelDeleteEventFlag(events[i].ctrl_eventid);
    ksceKernelDeleteEventFlag(events[i].ctrlext_eventid);

    events[i].unk_00 = 0;
    events[i].unk_04 = 0;

    char name[16];
    snprintf(name, sizeof(name), "SceCtrl%d", i);
    events[i].ctrl_eventid = ksceKernelCreateEventFlag(name, 0x9000, 0, NULL);
    snprintf(name, sizeof(name), "SceCtrlExt%d", i);
    events[i].ctrlext_eventid = ksceKernelCreateEventFlag(name, 0x9000, 0, NULL);
  }

  // Alloc per-process ctrl heaps
  int (* alloc_process_heap)(SceUID pid);
  module_get_offset(KERNEL_PID, tai_info.modid, 0, 0x5E9, (uintptr_t *)&alloc_process_heap);
  alloc_process_heap(KERNEL_PID);
  if (shell_pid != -1)
    alloc_process_heap(shell_pid);

  return 0;
}

static int get_module_info_func() {
  int res;

  res = module_get_export_func(KERNEL_PID, "SceKernelModulemgr", 0xC445FA63,
                               0xD269F915, (uintptr_t *)&_ksceKernelGetModuleInfo);
  if (res < 0)
    res = module_get_export_func(KERNEL_PID, "SceKernelModulemgr", 0x92C9FFC2,
                                 0xDAA90093, (uintptr_t *)&_ksceKernelGetModuleInfo);
  return res;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
  int res;

  res = get_module_info_func();
  if (res < 0)
    return SCE_KERNEL_START_FAILED;

  res = patch_bt();
  if (res < 0)
    return SCE_KERNEL_START_FAILED;

  res = patch_ctrl(ksceKernelSysrootGetShellPid());
  if (res < 0)
    return SCE_KERNEL_START_FAILED;

  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
  if (hooks[1] >= 0)
    taiHookReleaseForKernel(hooks[1], ksceSysrootCheckModelCapabilityRef);
  if (hooks[0] >= 0)
    taiInjectReleaseForKernel(hooks[0]);

  return SCE_KERNEL_STOP_SUCCESS;
}
