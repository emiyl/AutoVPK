/*

	Code modified by coderx3 and emiyl
	Based on VitaShell by TheFloW

  VitaShell
  Copyright (C) 2015-2018, TheFloW

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

#include <debugnet.h>
#include <psp2/display.h>
#include <psp2/promoterutil.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>
#include <psp2/ctrl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "file.h"
#include "graphics.c"
#define printf psvDebugScreenPrintf

#define ip_server "192.168.1.107"
#define port_server 18194

#include "headgen.h"
#include "file.h"


static int loadScePaf() {
  static uint32_t argp[] = { 0x180000, -1, -1, 1, -1, -1 };

  int result = -1;

  uint32_t buf[4];
  buf[0] = sizeof(buf);
  buf[1] = (uint32_t)&result;
  buf[2] = -1;
  buf[3] = -1;

  return sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(argp), argp, buf);
}

static int unloadScePaf() {
  uint32_t buf = 0;
  return sceSysmoduleUnloadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, 0, NULL, &buf);
}

// Name is used for temporary folder so don't put any weird strings in there !
void installApp(const char* srcFolder , const char* name ){
  // Start promoter stuff
  loadScePaf();
  sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
  scePromoterUtilityInit();

  const char *copyFolderRoot = "ux0:data/AutoVPK";

  char tmpFolder[255];
  snprintf(tmpFolder , 255 , "%s%s%s" , copyFolderRoot , hasEndSlash(copyFolderRoot) ? "" : "/", name);
  debugNetPrintf(DEBUG,"Copying from %s  to   %s \r\n" , srcFolder , tmpFolder );
  int copyResult = copyPath(srcFolder , tmpFolder);
  if(copyResult < 0){
    sceIoRmdir(tmpFolder);
    debugNetPrintf(DEBUG,"Failed copying with errorcode :  %d \n" , copyResult);
    // End promoter stuff
    scePromoterUtilityExit();
    sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    unloadScePaf();
    return;
  }
  debugNetPrintf(DEBUG,"Result of copy :  %d \n" , copyResult);

  debugNetPrintf(DEBUG,"Installing %s \n" , name);
  debugNetPrintf(DEBUG,"From %s \n" , srcFolder);
  debugNetPrintf(DEBUG,"Temporary folder %s \n" , tmpFolder);

  generateHeadBin(tmpFolder);
  if (scePromoterUtilityPromotePkgWithRif(tmpFolder, 1) == 0)
  {
    debugNetPrintf(DEBUG,"Successful install of %s \n" , name);
  }
  else
  {
    debugNetPrintf(DEBUG,"Failed to install %s \n" , name);
  }

  // End promoter stuff
  scePromoterUtilityExit();
  sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
  unloadScePaf();
  debugNetPrintf(DEBUG,"\r\n\r\n\r\n");
}

static unsigned buttons[] = {
	SCE_CTRL_SELECT,
	SCE_CTRL_START,
	SCE_CTRL_UP,
	SCE_CTRL_RIGHT,
	SCE_CTRL_DOWN,
	SCE_CTRL_LEFT,
	SCE_CTRL_LTRIGGER,
	SCE_CTRL_RTRIGGER,
	SCE_CTRL_TRIANGLE,
	SCE_CTRL_CIRCLE,
	SCE_CTRL_CROSS,
	SCE_CTRL_SQUARE,
};

int get_key(void) {
	static unsigned prev = 0;
	SceCtrlData pad;
	while (1) {
		memset(&pad, 0, sizeof(pad));
		sceCtrlPeekBufferPositive(0, &pad, 1);
		unsigned new = prev ^ (pad.buttons & prev);
		prev = pad.buttons;
		for (int i = 0; i < sizeof(buttons)/sizeof(*buttons); ++i)
			if (new & buttons[i])
				return buttons[i];

		sceKernelDelayThread(1000); // 1ms
	}
}

int main(int argc, char *argv[]) {
	psvDebugScreenInit();
  const char *version = "1.0";
  sceIoMkdir( "ux0:data/AutoVPK/" , 0777);
  // Start debugnet
  debugNetInit(ip_server,port_server,DEBUG);
  // Log start
  debugNetPrintf(DEBUG,"Started AutoVPK  V%s\n" , version);
  debugNetPrintf(DEBUG,"\r\n\r\n\r\n");

  // Source folders
  const char *vitashellFolder	= "app0:homebrew/VitaShell";
  const char *ensoFolder 	   	= "app0:homebrew/enso";
  const char *vhbbFolder 	   	= "app0:homebrew/VitaHBBrowser";

  // Things on screen
  printf("\n AutoVPK v"); printf(version); printf(" by emiyl and coderx3\n\n");
  printf(" This will install VitaShell, the Vita Homebrew Browser,\n and the HENkaku Enso Installer to your Vita\n\n");
  printf(" Press X to continue\n");
  printf(" Press O to cancel\n\n");

  switch (get_key()) {
    case SCE_CTRL_CROSS:
      goto go;
    case SCE_CTRL_CIRCLE:
      sceKernelExitProcess(0);
  }

go:
  // Install applications
  psvDebugScreenClear( COLOR_BLACK );
  printf("\n AutoVPK v"); printf(version); printf(" by emiyl and coderx3\n\n");
  printf(" Installing VitaShell...\n");
  installApp(vitashellFolder , "VitaShell");
  sceKernelDelayThread(1000*1000);
  printf(" Installing the HENkaku Enso Installer...\n");
  installApp(ensoFolder , "Enso Installer");
  sceKernelDelayThread(1000*1000);
  printf(" Installing the Vita Homebrew Browser...\n\n");
  installApp(vhbbFolder , "VitaHBBrowser");

  // Exit
  debugNetPrintf(DEBUG,"\r\n\r\nFinished tasks! \r\n");
  printf(" All tasks finished\n");
  printf(" Press any key to exit");
  get_key();

  // End debugNet
  debugNetFinish();

  sceIoRmdir("ux0:data/AutoVPK/");
  sceKernelExitProcess(0);
}
