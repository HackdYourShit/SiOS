//
//  Snes9xCallbacks.cpp
//  pixelbuffertest
//
//  Created by Lucas Menge on 1/5/12.
//  Copyright (c) 2012 Lucas Menge. All rights reserved.
//

#include "Snes9xCallbacks.h"

#include <sys/time.h>

#include "snes9x/snes9x.h"
#include "snes9x/memmap.h"
#include "snes9x/apu/apu.h"
#include "snes9x/controls.h"
#include "snes9x/display.h"

#include "Snes9xMain.h"

#pragma mark Defines

#undef TIMER_DIFF
#define TIMER_DIFF(a, b) ((((a).tv_sec - (b).tv_sec) * 1000000) + (a).tv_usec - (b).tv_usec)

#pragma mark - External Forward Declarations

extern SoundStatus so;

#pragma mark - SNES9X Callbacks

void S9xExit ()
{
}

const char * S9xGetDirectory (enum s9x_getdirtype dirtype)
{
  static int	index = 0;
  static char	path[4][PATH_MAX + 1];
  
  char	inExt[16];
  char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
  
  index++;
  if (index > 3)
    index = 0;
  
  switch (dirtype)
  {
    case SNAPSHOT_DIR:		strcpy(inExt, ".frz");	break;
    case SRAM_DIR:			strcpy(inExt, ".srm");	break;
    case SCREENSHOT_DIR:	strcpy(inExt, ".png");	break;
    case SPC_DIR:			strcpy(inExt, ".spc");	break;
    case CHEAT_DIR:			strcpy(inExt, ".cht");	break;
    case BIOS_DIR:			strcpy(inExt, ".bio");	break;
    case LOG_DIR:			strcpy(inExt, ".log");	break;
    default:				strcpy(inExt, ".xxx");	break;
  }
  
  _splitpath(S9xGetFilename(inExt, dirtype), drive, dir, fname, ext);
  _makepath(path[index], drive, dir, "", "");
  
  int	l = strlen(path[index]);
  if (l > 1)
    path[index][l - 1] = 0;
  
  return (path[index]);
}

bool8 S9xDoScreenshot (int width, int height)
{
  return true;
}

const char * S9xStringInput (const char *s)
{
  return (NULL);
}

void S9xHandlePortCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{
  return;
}

bool S9xPollAxis (uint32 id, int16 *value)
{
  return (false);
}

void S9xToggleSoundChannel (int c)
{
  static int	channel_enable = 255;
  
  if (c == 8)
    channel_enable = 255;
  else
    channel_enable ^= 1 << c;
  
  S9xSetSoundControl(channel_enable);
}

bool S9xPollButton (uint32 id, bool *pressed)
{
  *pressed = false;
  return true;
}

bool8 S9xContinueUpdate (int width, int height)
{
  return (true);
}

bool S9xPollPointer (uint32 id, int16 *x, int16 *y)
{
  *x = *y = 0;
  
  return (true);
}

const char * S9xChooseFilename (bool8 read_only)
{
  return (NULL);
}

const char * S9xChooseMovieFilename (bool8 read_only)
{
  return (NULL);
}

void S9xSetPalette ()
{
  
}

bool8 S9xOpenSnapshotFile (const char *fname, bool8 read_only, STREAM *file)
{
  if (read_only)
  {
    *file = OPEN_STREAM(fname,"rb");
    if (*file) 
      return(TRUE);
  }
  else
  {
    *file = OPEN_STREAM(fname,"w+b");
    if (*file) 
      return(TRUE);
  }
  
  return (FALSE);	
}

void S9xCloseSnapshotFile (STREAM file)
{
  CLOSE_STREAM(file);
}

void S9xMessage (int /* type */, int /* number */, const char *message)
{
  printf ("%s\n", message);
}

bool8_32 S9xInitUpdate ()
{
  //GFX.Screen = (uint8 *) framebuffer16 + (640*8) + 64;
  
  return (TRUE);
}

bool8 S9xDeinitUpdate (int width, int height)
{
  
  // TODO clear Z buffer if not in fastsprite mode
  SIFlipFramebuffer(0,0);
  
  return (TRUE);
}

const char *S9xGetFilename (const char *ex, enum s9x_getdirtype dirtype)
{
  static char filename [PATH_MAX + 1];
  char drive [_MAX_DRIVE + 1];
  char dir [_MAX_DIR + 1];
  char fname [_MAX_FNAME + 1];
  char ext [_MAX_EXT + 1];
  
  _splitpath (Memory.ROMFilename, drive, dir, fname, ext);
  strcpy (filename, SIGetSnapshotDirectory());
  strcat (filename, SLASH_STR);
  strcat (filename, fname);
  strcat (filename, ex);
  return (filename);
}

//const char *S9xGetFilenameInc (const char *e)
const char * S9xGetFilenameInc (const char *inExt, enum s9x_getdirtype dirtype)
{
  S9xMessage (0,0,"get filename inc");
  //return e;
  return inExt;
}

void S9xSyncSpeed(void)
{
  static int debt = 0;
  static struct timeval next_frame_time = { 0, 0 };
  struct timeval now;
  
  gettimeofday (&now, NULL);
  
  if (next_frame_time.tv_sec == 0)
  {
    next_frame_time = now;
    ++next_frame_time.tv_usec;
  }
  
  int lag = TIMER_DIFF (now, next_frame_time);
  debt += lag-(int)Settings.FrameTime;
  
  // if we're  going too fast
  if(debt < 0 && IPPU.SkippedFrames == 0)
  {
    usleep(-debt);
    debt = 0;
  }
  
  // if we're going too slow
  if (Settings.SkipFrames == AUTO_FRAMERATE && !Settings.SoundSync)
  {    
    if(debt > (int)Settings.FrameTime*10 || IPPU.SkippedFrames >= 2)
      debt = 0;
    
    if(debt > 0)
    {
      IPPU.RenderThisFrame = 0;
      IPPU.SkippedFrames++;
    }
    else
    {
      IPPU.RenderThisFrame = 1;
      IPPU.SkippedFrames = 0;
    }
  }
  //next_frame_time = now;
  gettimeofday (&next_frame_time, NULL);
}

const char *S9xBasename (const char *f)
{
  const char *p;
  
  S9xMessage (0,0,"s9x base name");
  
  if ((p = strrchr (f, '/')) != NULL || (p = strrchr (f, '\\')) != NULL)
    return (p + 1);
  
  return (f);
}

//};

//bool8_32 S9xOpenSoundDevice (int mode, bool8_32 stereo, int buffer_size)
bool8 S9xOpenSoundDevice (void) // HACK: this is never called, apparently
{
  so.sound_switch = 255;
  //so.playback_rate = mode;
  so.stereo = FALSE;//stereo;
  return TRUE;
}

void S9xAutoSaveSRAM (void)
{
	//since I can't sync the data, there is no point in even writing the data
	//out at this point.  Instead I'm now saving the data as the users enter the menu.
  SISaveSRAM();
	//sync();  can't sync when emulator is running as it causes delays
}

#pragma mark - OS-Related Path Manipulation

void _makepath (char *path, const char *, const char *dir,
                const char *fname, const char *ext)
{
	if (dir && *dir)
	{
		strcpy (path, dir);
		strcat (path, "/");
	}
	else
    *path = 0;
	strcat (path, fname);
	if (ext && *ext)
	{
		strcat (path, ".");
		strcat (path, ext);
	}
}

void _splitpath (const char *path, char *drive, char *dir, char *fname,
                 char *ext)
{
	*drive = 0;
  
	char *slash = strrchr (path, '/');
	if (!slash)
		slash = strrchr (path, '\\');
  
	char *dot = strrchr (path, '.');
  
	if (dot && slash && dot < slash)
		dot = NULL;
  
	if (!slash)
	{
		strcpy (dir, "");
		strcpy (fname, path);
		if (dot)
		{
			*(fname + (dot - path)) = 0;
			strcpy (ext, dot + 1);
		}
		else
			strcpy (ext, "");
	}
	else
	{
		strcpy (dir, path);
		*(dir + (slash - path)) = 0;
		strcpy (fname, slash + 1);
		if (dot)
		{
			*(fname + (dot - slash) - 1) = 0;
			strcpy (ext, dot + 1);
		}
		else
			strcpy (ext, "");
	}
}