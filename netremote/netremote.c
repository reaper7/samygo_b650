// ===========================================================================
// netremote v 0.1
// ===========================================================================
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "include/myplugin.h"
#include "include/netremote.h"
#include "include/rcbuttons.h"
#include "include/cributtons.h"
#include "include/popup.h"

extern unsigned int _ZN8SsObject14m_poObjectListE[];
extern int _ZN14SsKeyInputBase7SendKeyEi(unsigned int, int);

// ---------------------------------------------------------------------------
static struct keylist_t
{
  unsigned int samsung_code;
  unsigned int remote_code;
  unsigned int in_mode;
} keylist[] =
{
  {KEY_POWER,SAT_KEY_POWER,1,},
  {KEY_MUTE,SAT_KEY_MUTE,1,},

  //tv
  {KEY_CONTENT,SAT_KEY_TV_RADIO,2,},
  {KEY_INTERNET,SAT_KEY_WWW,1,},
  //sat

  //{KEY_SOURCE,SAT_KEY_PIP,1,},			//zamiana z KEY_FAV_CH			ORG
	//{KEY_SOURCE,SAT_KEY_FAVORITES,2,},	//zamiana z KEY_FAV_CH	NEW
  //pip_move    SAT_KEY_PIP_MOVE
  //pip_switch  SAT_KEY_PIP_SWITCH
  {KEY_CH_LIST,SAT_KEY_PIP_CH,1,},

  {KEY_P_UP,SAT_KEY_PAGEUP,1,},
  {KEY_PRE_CH,SAT_KEY_RECALL,1,},
  {KEY_VOL_PLUS,SAT_KEY_VOLUP,2,},

  {KEY_P_DOWN,SAT_KEY_PAGEDOWN,1,},
  {KEY_INFO,SAT_KEY_INFO,1,},
  {KEY_VOL_MINUS,SAT_KEY_VOLDOWN,2,},

  //{KEY_FAV_CH,SAT_KEY_FAVORITES,2,},	//zamiana z KEY_SOURCE			ORG
	{KEY_FAV_CH,SAT_KEY_PIP,1,},					//zamiana z KEY_SOURCE	NEW
  {KEY_TOOLS,SAT_KEY_CHECK,1,},
  {KEY_MEDIA_P,SAT_KEY_FILE,1,},
  {KEY_GUIDE,SAT_KEY_EPG,1,},

  {KEY_CUR_UP,SAT_KEY_UP,1,},

  {KEY_MENU,SAT_KEY_MENU,1,},
  {KEY_CUR_LEFT,SAT_KEY_LEFT,1,},
  {KEY_ENTER,SAT_KEY_OK,1,},
  {KEY_CUR_RIGHT,SAT_KEY_RIGHT,1,},
  {KEY_EXIT,SAT_KEY_BACK,1,},

  {KEY_CUR_DOWN,SAT_KEY_DOWN,1,},

  {KEY_COL_RED,SAT_KEY_RED,1,},
  {KEY_COL_GREEN,SAT_KEY_GREEN,1,},
  {KEY_COL_YELLOW,SAT_KEY_YELLOW,1,},
  {KEY_COL_BLUE,SAT_KEY_BLUE,1,},

  {KEY_NUM1,SAT_KEY_1,1,},
  {KEY_NUM2,SAT_KEY_2,1,},
  {KEY_NUM3,SAT_KEY_3,1,},

  {KEY_NUM4,SAT_KEY_4,1,},
  {KEY_NUM5,SAT_KEY_5,1,},
  {KEY_NUM6,SAT_KEY_6,1,},

  {KEY_NUM7,SAT_KEY_7,1,},
  {KEY_NUM8,SAT_KEY_8,1,},
  {KEY_NUM9,SAT_KEY_9,1,},

  //music
  {KEY_NUM0,SAT_KEY_0,1,},
  //photo

  {KEY_RECORD,SAT_KEY_RECORD,1,},
  {KEY_REWIND,SAT_KEY_REWIND,1,},
  {KEY_PLAY,SAT_KEY_PLAY,1,},
  {KEY_FORWARD,SAT_KEY_FASTFORWARD,1,},

  //repeat
  //slow
  {KEY_STOP,SAT_KEY_STOP,1,},
  {KEY_PAUSE,SAT_KEY_PAUSE,1,},

  //multi
  {KEY_SUBT,SAT_KEY_SUBTITLE,2,},
  {KEY_TTX_MIX,SAT_KEY_TELETEXT,2,},
  //audio

  {KEY_RETURN,SAT_KEY_BACK,1,},
  {KEY_TV,0,0,},
};
#define KEYLIST_NUM (sizeof(keylist) / sizeof(keylist[0]) )
// ---------------------------------------------------------------------------
int code2code(int code, int sel_mode)
{
  int i;
  for( i=0; i<KEYLIST_NUM; i++ )
    if ((keylist[i].samsung_code == code) && (sel_mode >= keylist[i].in_mode))
      return keylist[i].remote_code;
  return 0;
}
// ---------------------------------------------------------------------------
int send_key(int key_to_lan)
{
  int sockfd;
  int portno;
  char *hostname;
  char buf[3];
  struct hostent *server;
  struct sockaddr_in serveraddr;

  hostname = "192.168.0.3";
  portno = 2345;

  //socket: create the socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) printf("ERROR opening socket");
  //gethostbyname: get the server's DNS entry
  server = gethostbyname(hostname);
  if (server == NULL)
  {
    printf("ERROR, no such host as %s\n", hostname);
    return -1;
  }
  //build the server's Internet address
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
  serveraddr.sin_port = htons(portno);
  //connect: create a connection with the server
  if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0)
  {
    printf("ERROR connecting");
    return -1;
  }
  
  sprintf( buf, "%d", key_to_lan );
  
  if ((write(sockfd, buf, strlen(buf))) < 0)
  {
    printf("ERROR writing to socket");
    return -1;
  }
  close(sockfd);

  return 0;
}
// ---------------------------------------------------------------------------
void power_thread(void *pm)
{
  usleep(5500000);
  if (power_mode==2) 
  {
    remote_mode=0;
    power_mode=0;
    show_popup(PLUGNAME,remote_mode_text[remote_mode],2); 
  }
	return;
}
// ---------------------------------------------------------------------------
int key_finder(int pressed_key)
{
  if (remote_mode>0)
  {
    int transcode,send_res;
    //fl = fopen(logfile, "a");
    //fprintf(fl, "SAM_KEY:[%d]\n", pressed_key);
    //fflush(fl);
    transcode=code2code(pressed_key, remote_mode);
    if (transcode!=0)
    {
      send_res=send_key(transcode);
      if (send_res==0)
      {
        if (power_mode==2)
        {
          pthread_t pmode;
          pthread_create(&pmode, NULL, &power_thread, power_mode);
        }
        return (KEY_NOTHING);         
      }
      else
      {
        remote_mode=0;
        show_popup(PLUGNAME,remote_mode_text[remote_mode],2);
        return (KEY_NOTHING);
      }
    }
    //fclose(fl);
  }

  return (pressed_key);
}
// ---------------------------------------------------------------------------
void send_any_key(int kc)
{
	_ZN14SsKeyInputBase7SendKeyEi(_ZN8SsObject14m_poObjectListE[2], kc);
}
// ---------------------------------------------------------------------------
static int button_pressed(int a, int key, int arg_r2) __attribute__ ((noinline));
static int button_pressed(int a, int key, int arg_r2)
{
  switch (key)
  {
    case KEY_TV:
      if (remote_mode==max_modes)
      {
        remote_mode=0;
        power_mode=0;
				key=KEY_NOTHING;
      }
      else
			{
        remote_mode++;
				key=KEY_HDMI1;
			}
      show_popup(PLUGNAME,remote_mode_text[remote_mode],2);
    break;

    case KEY_POWER:
      if (remote_mode>0)
      {
        if (power_mode<=1) 
          power_mode++;
        key=key_finder(key); 
      }
      else
      {
        key=KEY_POWER;
      }
    break;

		case KEY_SOURCE:
			if (remote_mode>0)
				key=KEY_HDMI_TOGGLE;
			else
				key=KEY_SOURCE;
		break;

    default:
      if (remote_mode>0)
      {
        if (power_mode==0) 
          power_mode=1;
        else if (power_mode==2)
          power_mode=0;
        key=key_finder(key);
      }
    break;
  }

  return (key);
}

// ---------------------------------------------------------------------------

// universal injection procedure
void injection(int a, int key, int arg_r2) __attribute__ ((noinline));
void injection(int a, int key, int arg_r2)
{
        // CAUTION! Do not add/modify code here... or modify injection indexes in loader

        // [0][1] gcc should put "STMFD SP!, {R4,LR}" and "MOV R4,[R1]" here
        asm volatile(
                "STMFD   SP!, {R0,R2,R3,R12}\n"         // [2] store some registers
        );

        // first, we have to execute our function
        button_pressed(a, key, arg_r2);                         // [3][4] calling local function with key variable in R0, returning R0

        // then, prepare for calling and call previous injection function
        asm volatile(
                "MOV     R1, R0\n"                      // [5] remapped keycode
                "MOV     R6, R0\n"                      // [6] remapped keycode
                "LDMFD   SP!, {R0,R2,R3,R12}\n"         // [7] restore registers stored before
                "LDMFD   SP!, {R4,LR}\n"                // [8] restore registers...
                "MOV     R7, R2\n"                      // [9] This is original command that we overwrite (and replace at next injection)
                "LDR     R3, [R8]\n"                    // [10] This is original command that we overwrite (and replace at next injection)
                "LDR     PC, =(_ZN9KeyCommon17SendKeyPressInputEii + 0x18 )\n"  // [11] return after injection (not reached at next injection)
        );
        // and that's all; the rest (returning to exeDSP) is already programmed in the first injected function
}

// ---------------------------------------------------------------------------

// setting Game_Main path
void set_path(char *p)
{
   strcpy(path, p);
}

// ---------------------------------------------------------------------------
// end.

 
