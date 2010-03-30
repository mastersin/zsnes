#include <conio.h>
#include <stdio.h>
#include <string.h>

#include "../asm_call.h"
#include "../c_intrf.h"
#include "../c_vcache.h"
#include "../cfg.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../gui/gui.h"
#include "../input.h"
#include "../intrf.h"
#include "../link.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/c_2xsaiw.h"
#include "../video/newgfx16.h"
#include "c_winintrf.h"

#ifndef __RELEASE__
#	include "winlink.h"
#endif


u4 delayvalue;


void StartUp(void) {}


void SystemInit(void)
{
#ifndef __RELEASE__
	DisplayWIPDisclaimer();
#endif
	// Be sure to set SBHDMA to a value other than 0 if 16bit sound exists
	SBHDMA = 1;
}


void PrintChar(char const c)
{
	putchar(c);
}


void PrintStr(char const* const s)
{
	fputs(s, stdout);
}


char WaitForKey(void)
{
	return getch();
}


u1 Check_Key(void)
{
	return CurKeyPos != CurKeyReadPos ? 0xFF : 0;
}


char Get_Key(void)
{
	u4 const pos = CurKeyReadPos;
	while (CurKeyPos == pos) {} // XXX busy waiting
	if (KeyBuffer[pos] & 0x100)
	{
		KeyBuffer[pos] -= 0x100;
		return 0;
	}
	else
	{
		CurKeyReadPos = (pos + 1) % lengthof(KeyBuffer);
		return KeyBuffer[pos];
	}
}


void delay(u4 const n)
{
	delayvalue = n;
	DoSleep();
}


void InitPreGame(void)
{
	pressed[1] = 2;
	Start60HZ();

	initwinvideo();

	if (V8Mode != GrayscaleMode) V8Mode ^= 1;

	memset(vidbufferofsb, 0, 288 * 128 * 4);

	clearwin();

	// set up interrupt handler
	// get old handler pmode mode address
	// Process stuff such as sound init, interrupt initialization
}


void SetupPreGame(void)
{
	pressed[1] = 2;
}


void DeInitPostGame(void)
{
	Stop60HZ();
}


void GUIInit(void)
{
	Start36HZ();
}


void GUIDeInit(void)
{
	Stop36HZ();
}


static void InitializeGfxStuff(void)
{
	static u1 const BitPosR  = 11;
	static u1 const BitPosG  =  5;
	static u1 const BitPosB  =  0;
	static u1 const BitSizeR =  5;
	static u1 const BitSizeG =  6;
	static u1 const BitSizeB =  5;

	{ // Process Red Stuff
		u1 al = BitPosR;
		u2 bx = 1U << al;
		if (BitSizeR == 6)
		{
			vesa2_usbit = bx;
			++al;
		}
		vesa2_clbit |= bx;
		vesa2_rpos   = al;
		--al;
		vesa2_rfull = al != 0xFF ? 0x1FU << al : 0x1FU >> 1;
		bx = 1U << (al + 5);
		vesa2_rtrcl  =  bx;
		vesa2_rtrcla = ~bx;
	}

	{ // Process Green Stuff
		u1 al = BitPosG;
		u2 bx = 1U << al;
		if (BitSizeG == 6)
		{
			vesa2_usbit = bx;
			++al;
		}
		vesa2_clbit |= bx;
		vesa2_gpos   = al;
		--al;
		vesa2_gfull = al != 0xFF ? 0x1FU << al : 0x1FU >> 1;
		bx = 1U << (al + 5);
		vesa2_gtrcl  =  bx;
		vesa2_gtrcla = ~bx;
	}

	{ // Process Blue Stuff
		u1 al = BitPosB;
		u2 bx = 1U << al;
		if (BitSizeB == 6)
		{
			vesa2_usbit = bx;
			++al;
		}
		vesa2_clbit |= bx;
		vesa2_bpos   = al;
		--al;
		vesa2_bfull = al != 0xFF ? 0x1FU << al : 0x1FU >> 1;
		bx = 1U << (al + 5);
		vesa2_btrcl  =  bx;
		vesa2_btrcla = ~bx;
	}

	vesa2_clbit ^= 0xFFFF;
	genfulladdtab();
	Init_2xSaIMMX(converta != 1 ? 565 : 555);
}


void initvideo(void)
{
	static u4 firstvideo = 1;

	res640            =   1;
	res480            =   1;
	cbitmode          =   1;
	vesa2_x           = 512;
	vesa2_y           = 480;
	vesa2_bits        =  16;
	vesa2_rpos        =  11;
	vesa2_gpos        =   5;
	vesa2_bpos        =   0;
	vesa2red10        =   0;
	vesa2_rposng      =  11;
	vesa2_gposng      =   5;
	vesa2_bposng      =   0;
	vesa2_clbitng     = 0x0000F7DE;
	vesa2_clbitng2[0] = 0xF7DEF7DE;
	vesa2_clbitng2[1] = 0xF7DEF7DE;
	vesa2_clbitng3    = 0x00007BEF;

	initwinvideo();

	if (GUIWFVID[cvidmode] != 0)
		PrevFSMode = cvidmode;
	else
		PrevWinMode = cvidmode;

	if (firstvideo != 1)
		InitializeGfxStuff();
	firstvideo = 0;

	InitializeGfxStuff();
}


void deinitvideo(void) {}


void DrawScreen(void)
{
	if (converta == 1)
	{
		UnusedBit[0]    = 0x80008000;
		HalfTrans[0]    = 0x7BDE7BDE;
		UnusedBitXor[0] = 0x7FFF7FFF;
		UnusedBit[1]    = 0x80008000;
		HalfTrans[1]    = 0x7BDE7BDE;
		UnusedBitXor[1] = 0x7FFF7FFF;
		HalfTransB[0]   = 0x04210421;
		HalfTransB[1]   = 0x04210421;
		HalfTransC[0]   = 0x7BDE7BDE;
		HalfTransC[1]   = 0x7BDE7BDE;
		ngrposng        = 10;
		nggposng        =  5;
		ngbposng        =  0;
		ConvertToAFormat();
	}
	drawscreenwin();
}


void vidpastecopyscr(void)
{
	u1* const buf = vidbuffer;
	u4        n   = 224 * 288 - 288;
	u4        i   = 224 * 288 -   1;
	do ((u2*)buf)[i] = GUICPC[buf[i]]; while (--i, --n != 0);
	DrawScreen();
}


void UpdateDevices(void)
{
	WinUpdateDevices();
}


void JoyRead(void)
{
	UpdateVFrame();
}


#define SetDefaultKey2(player, k) \
  player##upk    = k[ 2], /* Up     */ \
  player##downk  = k[ 3], /* Down   */ \
  player##leftk  = k[ 4], /* Left   */ \
  player##rightk = k[ 5], /* Right  */ \
  player##startk = k[ 1], /* Start  */ \
  player##selk   = k[ 0], /* Select */ \
  player##Ak     = k[ 7], /* A      */ \
  player##Bk     = k[10], /* B      */ \
  player##Xk     = k[ 6], /* X      */ \
  player##Yk     = k[ 9], /* Y      */ \
  player##Lk     = k[ 8], /* L      */ \
  player##Rk     = k[11]  /* R      */

void SetInputDevice(u1 const device, u1 const player)
{
	// Sets keys according to input device selected
	static u1 const keys[][12] =
	{
		{  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
		{ 54, 28, 200, 208, 203, 205,  31,  45,  32,  30,  44,  46 },
		{ 56, 29,  36,  50,  49,  51, 210, 199, 201, 211, 207, 209 }
	};

	u1 const* const k =
		device == 0 ? keys[0] :
		player != 1 ? keys[1] :
		keys[2];

	switch (player)
	{
		case 0: SetDefaultKey2(pl1, k); break;
		case 1: SetDefaultKey2(pl2, k); break;
		case 2: SetDefaultKey2(pl3, k); break;
		case 3: SetDefaultKey2(pl4, k); break;
		case 4: SetDefaultKey2(pl5, k); break;
	}
}

#undef SetDefaultKey2


/*****************************
 * Mouse Stuff
 *****************************/


u4 Init_Mouse(void)
{
	return 1;
}


u4 Get_MouseData(void)
{
	u4 const x       = GetMouseX();
	u4 const y       = GetMouseY();
	u4 const buttons = GetMouseButton();
	return y << 24 | x << 16 | buttons;
}


void Set_MouseXMax(u4 const min, u4 const max)
{
	SetMouseMinX(min);
	SetMouseMaxX(max);
}


void Set_MouseYMax(u4 const min, u4 const max)
{
	SetMouseMinY(min);
	SetMouseMaxY(max);
}


void Set_MousePosition(u4 const x, u4 const y)
{
	SetMouseX(x);
	SetMouseY(y);
}


u4 Get_MousePositionDisplacement(void)
{
	u4 const x = GetMouseMoveX();
	u4 const y = GetMouseMoveY();
	return y << 16 | x;
}


void MouseWindow(void)
{
	MouseButton  |= 0x02;
	T36HZEnabled  = 1;
	GetMouseButton();
	MouseButton  &= 0xFD;
}


void StopSound(void)
{
	Start36HZ();
	JoyRead();
}


void StartSound(void)
{
	Start60HZ();
	JoyRead();
}


void SoundProcess(void)
{
	if (soundon != 0 && DSPDisable != 1)
	{
		BufferSizeB = 256; // Size
		BufferSizeW = 512;
		asm_call(ProcessSoundBuffer);
		// DSPBuffer should contain the processed buffer in the specified size
		// You will have to convert/clip it to 16-bit for actual sound process
	}
}


void Check60hz(void)
{
	CheckTimers();
	FrameSemaphore();
}


char const GUIVideoModeNames[][18] =
{
	"256x224       R W", //  0
	"256x224       R F", //  1
	"512x448       R W", //  2
	"512x448      DR W", //  3
	"640x480       S W", //  4
	"640x480      DS W", //  5
	"640x480      DR F", //  6
	"640x480      DS F", //  7
	"640x480       S F", //  8
	"768x672       R W", //  9
	"768x672      DR W", // 10
	"800x600       S W", // 11
	"800x600      DS W", // 12
	"800x600       S F", // 13
	"800x600      DR F", // 14
	"800x600      DS F", // 15
	"1024x768      S W", // 16
	"1024x768     DS W", // 17
	"1024x768      S F", // 18
	"1024x768     DR F", // 19
	"1024x768     DS F", // 20
	"1024x896      R W", // 21
	"1024x896     DR W", // 22
	"1280x960      S W", // 23
	"1280x960     DS W", // 24
	"1280x960      S F", // 25
	"1280x960     DR F", // 26
	"1280x960     DS F", // 27
	"1280x1024     S W", // 28
	"1280x1024    DS W", // 29
	"1280x1024     S F", // 30
	"1280x1024    DR F", // 31
	"1280x1024    DS F", // 32
	"1600x1200     S W", // 33
	"1600x1200    DS W", // 34
	"1600x1200    DR F", // 35
	"1600x1200    DS F", // 36
	"1600x1200     S F", // 37
	"CUSTOM       D  W", // 38
	"CUSTOM       DS F", // 39
	"CUSTOM          W", // 40
	"CUSTOM        S F", // 41
	"CUSTOM       DR F", // 42
	"512x448     ODR W", // 43
	"640x480     ODS F", // 44
	"640x480     ODS W", // 45
	"768x672     ODR W", // 46
	"800x600     ODS F", // 47
	"800x600     ODS W", // 48
	"1024x768    ODS F", // 49
	"1024x768    ODS W", // 50
	"1024x896    ODR W", // 51
	"1280x960    ODS F", // 52
	"1280x960    ODS W", // 53
	"1280x1024   ODS F", // 54
	"1280x1024   ODS W", // 55
	"1600x1200   ODS F", // 56
	"1600x1200   ODS W", // 57
	"CUSTOM      OD  F", // 58
	"VARIABLE    OD  W"  // 59
};

u4 const NumVideoModes = lengthof(GUIVideoModeNames);

#define _ 0
#define X 1
/* Video Mode Feature Availability (X = Available, _ = Not Available)
 * Left side starts with Video Mode 0
 *                 0                   1                   2                   3                   4                   5
 *                 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 */
u1 GUIBIFIL[]  = { _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X };
u1 GUIDSIZE[]  = { _,_,_,X,_,X,X,X,_,_,X,_,X,_,X,X,_,X,_,X,X,_,X,_,X,_,X,X,_,X,_,X,X,_,X,X,X,_,X,X,_,_,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X };
u1 GUIDSMODE[] = { _,_,_,_,_,X,_,X,_,_,_,_,X,_,_,X,_,X,_,_,X,_,_,_,X,_,_,X,_,X,_,_,X,_,X,_,X,_,X,X,_,_,_,_,X,X,_,X,X,X,X,_,X,X,X,X,X,X,X,X };
u1 GUIHQ2X[]   = { _,_,_,X,_,X,X,X,_,_,X,_,X,_,X,X,_,X,_,X,X,_,X,_,X,_,X,X,_,X,_,X,X,_,X,X,X,_,X,X,_,_,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X };
u1 GUIHQ3X[]   = { _,_,_,_,_,_,_,_,_,_,X,_,X,_,X,X,_,X,_,X,X,_,X,_,X,_,X,X,_,X,_,X,X,_,X,X,X,_,X,X,_,_,X,_,_,_,X,X,X,X,X,X,X,X,X,X,X,X,X,X };
u1 GUIHQ4X[]   = { _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,X,_,X,_,X,X,_,X,_,X,X,_,X,X,X,_,X,X,_,_,X,_,_,_,_,_,_,_,_,X,X,X,X,X,X,X,X,X };
u1 GUIKEEP43[] = { _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,X,X,X,_,X,_,_,_,_,_,X,X,X,X,_,_,_,_,_,_,_,_,_,_,_,_,X,X,_,_,X,X };
u1 GUIM7VID[]  = { _,_,_,X,_,X,X,X,_,_,X,_,X,_,X,X,_,X,_,X,X,_,X,_,X,_,X,X,_,X,_,X,X,_,X,X,X,_,X,X,_,_,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X };
u1 GUINTVID[]  = { _,_,_,_,_,X,X,X,_,_,X,_,X,_,X,X,_,X,_,X,X,_,X,_,X,_,X,X,_,X,_,X,X,_,X,X,X,_,X,X,_,_,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X };
u1 GUISMODE[]  = { _,_,_,_,X,_,_,_,X,_,_,X,_,X,_,_,X,_,X,_,_,_,_,X,_,X,_,_,X,_,X,_,_,X,_,_,_,X,_,_,X,X,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_ };
u1 GUIWFVID[]  = { _,X,_,_,_,_,X,X,X,_,_,_,_,X,X,X,_,_,X,X,X,_,_,_,_,X,X,X,_,_,X,X,X,_,_,X,X,X,_,X,_,X,X,_,X,_,_,X,_,X,_,_,X,_,X,_,X,_,X,_ };
#undef X
#undef _


char const GUIInputNames[][17] =
{
	"NONE            ",
	"KEYBOARD/GAMEPAD",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                "
};

u4 const NumInputDevices = 2;
