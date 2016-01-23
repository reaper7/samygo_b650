// ===========================================================================
// mybuttons v 0.7
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
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "include/myplugin.h"
#include "include/mybuttons.h"
#include "include/rcbuttons.h"
#include "include/popup.h"

extern int spIDp_Open(int, unsigned int *);
extern int spIDp_Close(unsigned int);
extern int spIDp_SetFreeze(unsigned int, int);

extern unsigned int _ZN8SsObject14m_poObjectListE[];
extern int _ZN14SsKeyInputBase7SendKeyEi(unsigned int, int);

// ---------------------------------------------------------------------------
int hide_window(void)
{
	if (flag_screen==1) {
		if (flag_font!=0) {
			TTF_CloseFont(font);
			TTF_Quit();
			flag_font=0;
		}
		SDL_FillRect(screen, &dstrect, 0);
		SDL_Flip(screen);
		flag_screen=0;
		SDL_FreeSurface(screen);
	}
	return (KEY_NOTHING);
}
// ---------------------------------------------------------------------------
void clean_window(void)
{
	SDL_FillRect(screen, &dstrect, 0);
}
// ---------------------------------------------------------------------------
int show_window(void)
{
	char uni_path[256], text_tmp[256];
	struct stat buf;

	//loading image
	sprintf(uni_path, "%s/%s", path, SC_BUTTONSBMP);
	if (stat(uni_path, &buf)==0) {
		sc_buttons = IMG_Load(uni_path);
		if (sc_buttons) SDL_BlitSurface(sc_buttons, NULL, screen, &dstrect);
		SDL_FreeSurface(sc_buttons);
	}

	if (flag_font==0) {
		if (TTF_Init()!=0) return 1;
		sprintf(uni_path, SC_FONT);
		if (stat(uni_path, &buf)!=0) return 1;
		font=TTF_OpenFont(uni_path, FONT_SIZE);
		if (!font) return 1;
		flag_font=1;
	}
	
	if (sprintf(text_tmp,my_config[poscur*4+RED].bcaption)>0) {
		rtext_fg = TTF_RenderUTF8_Blended(font, text_tmp, white);
		SDL_BlitSurface(rtext_fg, &rtsrect_fg, screen, &rtdrect);
		SDL_FreeSurface(rtext_fg);
	}
	if (sprintf(text_tmp,my_config[poscur*4+GREEN].bcaption)>0) {
		gtext_fg = TTF_RenderUTF8_Blended(font, text_tmp, white);
		SDL_BlitSurface(gtext_fg, &gtsrect_fg, screen, &gtdrect);
		SDL_FreeSurface(gtext_fg);
	}
	if (sprintf(text_tmp,my_config[poscur*4+YELLOW].bcaption)>0) {
		ytext_fg = TTF_RenderUTF8_Blended(font, text_tmp, white);
		SDL_BlitSurface(ytext_fg, &ytsrect_fg, screen, &ytdrect);
		SDL_FreeSurface(ytext_fg);
	}
	if (sprintf(text_tmp,my_config[poscur*4+BLUE].bcaption)>0) {
		btext_fg = TTF_RenderUTF8_Blended(font, text_tmp, white);
		SDL_BlitSurface(btext_fg, &btsrect_fg, screen, &btdrect);
		SDL_FreeSurface(btext_fg);
	}

	//make popup window transparent
	int x, y;																																						
	Uint32 *pixels = (Uint32 *)screen->pixels;
	Uint32 newPixel;
	for( x = dstrect.x; x < (dstrect.x+dstrect.w); x++ ) {
		for (y = dstrect.y; y < (dstrect.y+dstrect.h); y++) {
			Uint8 r;
			Uint8 g;
			Uint8 b;
			Uint8 a;
			SDL_LockSurface(screen);
			Uint32 onePixel = pixels[ ( y * screen->w ) + x ]; 
			SDL_GetRGBA(onePixel, screen->format, &r, &g, &b, &a);
			if ((r==0xff) && (g==0x00) && (b==0xff)) {
				a = 0;
			} else {
				a = 220;	
			}
			newPixel = SDL_MapRGBA(screen->format, r, g, b, a);
			pixels[ ( y * screen->w ) + x ] = newPixel;   
			SDL_UnlockSurface(screen);
		}
	}

	SDL_Flip(screen);
	return 0;
}
// ---------------------------------------------------------------------------
int make_screen(void)
{
	screen = SDL_GetVideoSurface();

	if (screen) {
		SDL_FreeSurface(screen);
		SDL_Quit();
		SDL_Init(SDL_INIT_VIDEO);
		SDL_SetVideoMode(SCREEN_W, SCREEN_H, SCREEN_BPP, SCREEN_FLAGS);
		screen = SDL_GetVideoSurface();
		SDL_Flip(screen);
	} else {
		screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, SCREEN_BPP, SCREEN_FLAGS);
	}

	if (screen) {
		flag_screen=1;
	} else {
		flag_screen=0;
	}
	return 0;
}
// ---------------------------------------------------------------------------
int read_config(void)
{
	char conf_path[255];	
	FILE *cf;
	int posr=0, posg=0, posy=0, posb=0;
	char linebuf[512], *stmp;

	char buttonname[10];
	char buttoncaption[50];
	char buttoncommand[255];

	memset(&my_config, 0, sizeof(my_config)); 																															//clean config last loaded to memory

	//loading config
	sprintf(conf_path, "%s/%s", path, CONF_FILE);
	cf = fopen(conf_path,"r");																																							//open config file
	if (!cf) return 1;
	while(fgets(linebuf, sizeof linebuf, cf) != NULL)																												//read config file line by line
	{
		if(((stmp = strchr(linebuf,'\n')) != NULL) && (strncmp(linebuf,"#",1)!=0)) {													
			*stmp = '\0';
			memset(&buttonname, 0, sizeof(buttonname));																													//clean temporary readed button color
			memset(&buttoncaption, 0, sizeof(buttoncaption));																										//clean temporary readed button caption
			memset(&buttoncommand, 0, sizeof(buttoncommand));																										//clean temporary readed button command
			if (3==sscanf(linebuf,"%10[^|]|%50[^|]|%255c",buttonname, buttoncaption, buttoncommand)) {					//if line is properly |NAME|CAPTION|COMMAND
				if (strcmp(buttonname,"RED")==0) {																																//line with RED button
					strcat(my_config[posr*4+RED].bcaption,buttoncaption);
					strcat(my_config[posr*4+RED].bcommand,buttoncommand);
					++posr;
					if (posr>posmax) posmax=posr;
				} else if (strcmp(buttonname,"GREEN")==0) {																												//line with GREEN button
					strcat(my_config[posg*4+GREEN].bcaption,buttoncaption);
					strcat(my_config[posg*4+GREEN].bcommand,buttoncommand);
					++posg;
					if (posg>posmax) posmax=posg;
				} else if (strcmp(buttonname,"YELLOW")==0) {																											//line with YELLOW button
					strcat(my_config[posy*4+YELLOW].bcaption,buttoncaption);
					strcat(my_config[posy*4+YELLOW].bcommand,buttoncommand);
					++posy;
					if (posy>posmax) posmax=posy;
				} else if (strcmp(buttonname,"BLUE")==0) {																												//line with BLUE button
					strcat(my_config[posb*4+BLUE].bcaption,buttoncaption);
					strcat(my_config[posb*4+BLUE].bcommand,buttoncommand);
					++posb;
					if (posb>posmax) posmax=posb;
				} else if (strcmp(buttonname,"BUTTON")==0) {																											//line with START button
					if (atoi(buttoncommand)!=0) start_button=atoi(buttoncommand);
				} else if (strcmp(buttonname,"PAUSE")==0) {																												//line with PAUSE button
					if (atoi(buttoncommand)!=0) pause_button=atoi(buttoncommand);
				} else if (strcmp(buttonname,"POPUP")==0) {																												//line with POPUP button
					if (atoi(buttoncommand)!=0) {
						popup_msg=atoi(buttoncommand);
						if (popup_msg > 10) {
							popup_msg=10;
						} else if (popup_msg < 0) {
							popup_msg=10;	
						}
					} else {
						popup_msg=0;			
					}
				}
			}
		}
	}
	fclose(cf);
	if (posmax==0) return 1;
	--posmax;
	poscur=0;
	return 0;	
}
// ---------------------------------------------------------------------------
int start_mybutton(void)
{
	char old_path[256];
	if (getcwd(old_path, sizeof(old_path)) == NULL) strcpy(old_path, "/mtd_exe/");

	chdir(path);
	setenv("HOME", path, 1);

	if (read_config()==0) {
		make_screen();
		if (flag_screen==1) show_window();
	}
	chdir(old_path);
	setenv("HOME", old_path, 1);
	return (KEY_RETURN);
}
// ---------------------------------------------------------------------------
static int _dp_toggle_freeze(int dpInst) {																																//SBAV1 PAUSE CODE
	//todo - text on screen "PAUSE ON" / "PAUSE OF"
	unsigned int hDp=0;
   
	int retv=spIDp_Open(dpInst, &hDp);
	if (retv != 0 || hDp == 0) return(-1);

	unsigned int *hDp_ptr=(unsigned int *)hDp;

	int pff=*(hDp_ptr+0x140/4);
	int nff=(pff == 0)? 1: 0;

	retv=spIDp_SetFreeze(hDp, nff);
	retv=spIDp_Close(hDp);

   return(0);
}
// ---------------------------------------------------------------------------
void macro_thread(void *col) {
	flag_macro=1;
	int n_rckey = 0;
	char *nextrckey;
	char linebuf[512];
	char keys_interval[10];
	char keys_codes[500];
	int slp;
	int kc;
	int max_array=0;

	usleep(750000);

	memset(&linebuf, 0, sizeof(linebuf));
	memset(&keys_interval, 0, sizeof(keys_interval));
	memset(&keys_codes, 0, sizeof(keys_codes));

	strcat(linebuf,my_config[poscur*4+(int)col].bcommand);

	fl = fopen(logfile, "a");
	fprintf(fl, "KEYS_MACRO\n[%d][%d] LINE[%s]\n", poscur, col, linebuf);
	fflush(fl);

	while (strcmp(rc_codes[max_array],"END")!=0) max_array++;

	if (2==sscanf(linebuf,"#%10[^#]#%255c",keys_interval, keys_codes)) {
		slp=(atoi(keys_interval)*1000);
		if (slp < 500000) {
			slp=DEF_SLEEP;
		} else if (slp > 5000000) {
			slp=5000000;
		}

		fprintf(fl, "KEYS_MACRO INTERVAL(usec) [%d]\n", slp);
		fflush(fl);

		for (nextrckey = strtok(keys_codes, "."); n_rckey < CMDMAX && nextrckey; nextrckey = strtok(NULL, ".")) {
			kc=0;
			while ((strcmp(rc_codes[kc],nextrckey)!=0) && (strcmp(rc_codes[kc],"END")!=0)) kc++;
			if (kc<max_array) {
				_ZN14SsKeyInputBase7SendKeyEi(_ZN8SsObject14m_poObjectListE[2], kc);
				fprintf(fl, "KEYS_MACRO FOUND RC_KEY [%d] -> [%d][%s]\n", n_rckey, kc, rc_codes[kc]);
				fflush(fl);
				usleep(slp);
			} else {
				fprintf(fl, "KEYS_MACRO UNKNOWN RC_KEY [%d] -> [%s]\nBREAK\n", n_rckey, nextrckey);
				fflush(fl);
				break;
			}
			n_rckey++;
		}
	} else {
		fprintf(fl, "KEYS_MACRO LINE ERROR, CORRECT FORMAT: #interval_in_ms#key_name1.key_name2.key_nameX\nBREAK\n");
		fflush(fl);
	}	
	fclose(fl);
	flag_macro=0;
	return;
}
// ---------------------------------------------------------------------------
void exe_command(int col)
{
	if (strncmp(my_config[poscur*4+col].bcommand,"#",1)!=0) {																				//detect COMMAND != 0 or KEYS_MACRO == 0
		fl = fopen(logfile, "a");																																			//open log file
		FILE *fcom;
	  char line[80];
		char answ[2040];
		memset(&answ, 0, sizeof(answ));
		if (!(fcom = popen(my_config[poscur*4+col].bcommand, "r"))) {
			if (popup_msg!=0) show_popup(PLUGNAME,"Error executing command!",popup_msg);
			fprintf(fl, "Error executing command!\n");
			fflush(fl);
		} else {
			fprintf(fl, "COMMAND\n[%d][%d] *%s*\nRESULT\n", poscur, col, my_config[poscur*4+col].bcommand);
			fflush(fl);
			while ((fgets(line, sizeof(line), fcom)) && (strlen(answ) < sizeof(answ))) strcat(answ,line);	
			if (strlen(answ)>0) {
				if (popup_msg!=0) show_popup(PLUGNAME,answ,popup_msg);
				fprintf(fl, "%s\n", answ);
				fflush(fl);
			}
			pclose(fcom);
		}
		fclose(fl);
	} else {																																												//keys_macro
		if (flag_macro==0) {
			pthread_t helper;
    	pthread_create(&helper, NULL, &macro_thread, col);
		} else {
			if (popup_msg!=0) show_popup(PLUGNAME,"another macro is in progress...",popup_msg);
		}
	}
} 
// ---------------------------------------------------------------------------
static int button_pressed(int a, int key, int arg_r2) __attribute__ ((noinline));
static int button_pressed(int a, int key, int arg_r2)
{
	if (flag_screen==0) {
		if (start_button==0) {
			read_config();
			if (start_button==0) start_button=KEY_COL_GREEN;
		}

		if (key==start_button) {
			key=start_mybutton();
		}
	} else {
		switch (key) {
			case KEY_COL_RED:
				key=hide_window();
				exe_command(RED);
			break;

			case KEY_COL_GREEN:
				key=hide_window();
				exe_command(GREEN);
			break;

			case KEY_COL_YELLOW:
				key=hide_window();
				exe_command(YELLOW);
			break;

			case KEY_COL_BLUE:
				key=hide_window();
				exe_command(BLUE);
			break;

			case KEY_CUR_RIGHT:
					if (poscur==posmax) {
						poscur=0;
					} else ++poscur;
					clean_window();
					show_window();
			break;

			case KEY_CUR_LEFT:
					if (poscur==0) {
						poscur=posmax;
					} else --poscur;
					clean_window();
					show_window();
			break;

			case KEY_RETURN:
				key=hide_window();
			break;

			case KEY_EXIT:
				key=hide_window();
			break;

			default:
				hide_window();
			break;
		}
	}
	
	if ((pause_button!=0) && (key==pause_button)) {
		_dp_toggle_freeze(0);
		key=KEY_NOTHING;
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
		"STMFD   SP!, {R0,R2,R3,R12}\n"		// [2] store some registers
	);

	// first, we have to execute our function
	button_pressed(a, key, arg_r2);				// [3][4] calling local function with key variable in R0, returning R0

	// then, prepare for calling and call previous injection function
	asm volatile(
		"MOV     R1, R0\n"			// [5] remapped keycode
		"MOV     R6, R0\n"			// [6] remapped keycode
		"LDMFD   SP!, {R0,R2,R3,R12}\n"		// [7] restore registers stored before
		"LDMFD   SP!, {R4,LR}\n"		// [8] restore registers...
		"MOV     R7, R2\n"			// [9] This is original command that we overwrite (and replace at next injection)
		"LDR     R3, [R8]\n"			// [10] This is original command that we overwrite (and replace at next injection)
		"LDR     PC, =(_ZN9KeyCommon17SendKeyPressInputEii + 0x18 )\n"	// [11] return after injection (not reached at next injection)
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

