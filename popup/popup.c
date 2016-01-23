#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "popup.h"

// --------------------------------------------------------------------

#define POPUP_SCREEN_W	(1920/2)																															//screen surface resolution: x
#define POPUP_SCREEN_H	(1080/2)																															//screen surface resolution: y
#define POPUP_SCREEN_BPP	32																																	//screen bits-per-pixel value
#define POPUP_SCREEN_FLAGS	SDL_HWSURFACE																											//screen flags now is SDL_HWSURFACE ; (0 or maybe SDL_SWSURFACE ?)

//#define POPUP_BG_BMP	"/mtd_appdata/Images_960x540//SingleBlue/AppList/appList_bg.bmp"				//background bmp file		
//#define POPUP_BG_BMP	"/mtd_appdata/Images_960x540//SingleBlue//SingleCommon/background02.bmp"				//background bmp file	
#define POPUP_BG_BMP	"/mtd_appdata/Images_960x540//SingleBlue//SingleCommon/Tr_background02.bmp"				//background bmp file	
#define POPUP_FONT_TTF	"/mtd_appdata/Font/shadow.ttf"																				//text ttf file			
	
#define	POPUP_FONT_SIZE	14																																		//font size
#define LINE_SPACE	1
#define LINE_Y_STEP	(POPUP_FONT_SIZE + LINE_SPACE)																						//line space

//#define IMG_W	250																																						//width  bmp selected above 
//#define IMG_H	177																																						//height bmp selected above
#define IMG_W	530																																							//width  bmp selected above 
#define IMG_H	297																																							//height bmp selected above
#define V_MARGIN	80																																					//top and bottom margin
#define ABS_IMG_X	((POPUP_SCREEN_W / 2) - (IMG_W / 2))																				//image x position on screen (for center)
//#define ABS_IMG_Y	((POPUP_SCREEN_H / 2) - (IMG_H / 2))																			//image y position on screen (for center)
#define ABS_IMG_Y	V_MARGIN																																		//image y position on screen
#define IMG_BORDER	4																																					//border on image file (if exists) - don't write text on it
#define START_Y_CAPT	(ABS_IMG_Y + 12)																												//caption y position on screen
#define START_Y_TEXT	(ABS_IMG_Y + 39)																												//text y position on screen
#define TEXT_AREA_MAX_W	(IMG_W - (IMG_BORDER * 2))																						//maximal width of text area inside image (both for caption and text)
#define TEXT_AREA_MAX_X	(ABS_IMG_X + IMG_BORDER + TEXT_AREA_MAX_W)														//maximal text x position on screen
//#define TEXT_AREA_MAX_Y	(ABS_IMG_Y + IMG_H - 2)																							//maximal text y position on screen
#define TEXT_AREA_MAX_Y	(POPUP_SCREEN_H - V_MARGIN - IMG_BORDER)															//maximal text y position on screen
#define MAX_POPUP_H	(POPUP_SCREEN_H - (V_MARGIN * 2))																					//maximal popup h size

SDL_Surface *popup_wnd;
SDL_Color popup_white = {255,255,255};
SDL_Color popup_grey = {172,172,172};

char *popup_caption;
char *popup_text;
int popup_timeout=0;

int is_write=0;
int i=0;

// --------------------------------------------------------------------

void* _show_popup(void* unused)
{
	SDL_Rect popuprect = {ABS_IMG_X, ABS_IMG_Y, IMG_W, MAX_POPUP_H};
	SDL_Rect imgrect = {0, 0, IMG_W, (START_Y_TEXT-ABS_IMG_Y)};
	SDL_Rect textrect_in = {0,0,TEXT_AREA_MAX_W, LINE_Y_STEP};
	SDL_Rect textrect_out = {(ABS_IMG_X + IMG_BORDER), START_Y_CAPT, TEXT_AREA_MAX_W, LINE_Y_STEP};
	SDL_Surface *popup_background, *pop_text;
	TTF_Font *popup_font;
	char tmp[2048];
	struct stat buf;
	int text_w,text_h,text_pos=0;
	char ch[3];
	char *p_caption;
	char *p_text;
	int p_timeout=0;

	p_caption=popup_caption;
	p_text=popup_text;
	p_timeout=popup_timeout*10;																																		//for 1 sec -> 10 * 100ms

	//pthread_t my_id;
	//my_id=pthread_self();

	//FILE *pop;
	//const char *popfile = "/dtv/usb/sda/popup.log";
	//pop = fopen(popfile, "a");
	//fprintf(pop, "POPUP -> %d - %ld with: FLAG=%d TIME=%d\n", i, (long int)my_id, flag_popup, p_timeout);
	//fflush(pop);

	if (p_timeout!=0) {																																					//if fimeout != 0 then paint, else go to clean window	
		while (is_write==1) {																																			//wait until other thread painting window
			SDL_Delay(100);	
		}
		is_write=1;																																								//set flag "i'm painting"
		SDL_FillRect(popup_wnd, &popuprect, 0);																										//clear popup window
		//loading background
		sprintf(tmp, POPUP_BG_BMP);
		if (stat(tmp, &buf)==0) {																																	//if background bmp exists load it
			popup_background = SDL_LoadBMP(tmp);
			if (popup_background) SDL_BlitSurface(popup_background, &imgrect, popup_wnd, &popuprect);
		}

		//loading font & write text
		if (TTF_Init()==0) {																																			//load SDL_ttf module
			sprintf(tmp, POPUP_FONT_TTF);
			if (stat(tmp, &buf)==0) {																																//if fontfile ttf exists load it
				popup_font=TTF_OpenFont(tmp, POPUP_FONT_SIZE);
				//caption
				if (p_caption!=NULL) {
					sprintf(tmp,p_caption);
					TTF_SetFontStyle(popup_font, TTF_STYLE_BOLD);
					pop_text = TTF_RenderUTF8_Blended(popup_font, tmp, popup_grey);

					TTF_SizeUTF8(popup_font,tmp,&text_w,&text_h);																				//get caption text size
					if (text_w>TEXT_AREA_MAX_W) {																												//calculate x position for text center align
						textrect_out.x=(ABS_IMG_X + IMG_BORDER);
					} else {
						textrect_out.x=((POPUP_SCREEN_W / 2) - (text_w / 2));
					}
					SDL_BlitSurface(pop_text, &textrect_in, popup_wnd, &textrect_out);
					SDL_FreeSurface(pop_text);
				}
				
				//text
				if (p_text!=NULL) {
					sprintf(tmp,p_text);
					TTF_SetFontStyle(popup_font, TTF_STYLE_NORMAL);

					TTF_SizeUTF8(popup_font,tmp,&text_w,&text_h);																				//check text size
					if ((text_w<=TEXT_AREA_MAX_W) && (strchr(tmp, '\n')==NULL)) {												//if text width is lower that one line width then set center position on text area
						textrect_out.x=((POPUP_SCREEN_W / 2) - (text_w / 2));															//set x to text center position
						textrect_out.y=(START_Y_TEXT+IMG_BORDER);																		//set y to text start position
						if (popup_background) {
							imgrect.y=(IMG_H-(IMG_BORDER * 2));
							imgrect.h=IMG_BORDER;
							popuprect.y=START_Y_TEXT;
							popuprect.h=IMG_BORDER;
							SDL_BlitSurface(popup_background, &imgrect, popup_wnd, &popuprect);
						}
					} else {
						textrect_out.x=(ABS_IMG_X + IMG_BORDER);																					//set x to text start position
						textrect_out.y=START_Y_TEXT;																											//set y to text start position
					}

					while(tmp[text_pos] != '\0')																												//get char by char until not text end 
		   			{

						if ((popup_background) && (popuprect.y!=textrect_out.y)) {
							imgrect.y=(IMG_H-LINE_Y_STEP-IMG_BORDER);
							imgrect.h=LINE_Y_STEP;
							popuprect.y=textrect_out.y;
							popuprect.h=LINE_Y_STEP;
							SDL_BlitSurface(popup_background, &imgrect, popup_wnd, &popuprect);
						}

						ch[0] = tmp[text_pos];

						if ((ch[0]-'0')==(-0x26)) {																												//skip to next line when char is \n
							textrect_out.y+=LINE_Y_STEP;
							if (textrect_out.y>TEXT_AREA_MAX_Y) break;																			//if "next line" is outside popup frame then break
							textrect_out.x=(ABS_IMG_X + IMG_BORDER);
							text_pos++;
						} else {
							if ((ch[0]-'0'>=0x93) && (ch[0]-'0'<=0x95)) {																		//UTF8 ? Polish UTF8 chars detect OK							
								ch[1] = tmp[text_pos+1];							
								ch[2] = '\0';
								text_pos+=2;
							} else {
								ch[1] = '\0';
								text_pos++;
							}

							TTF_SizeUTF8(popup_font,ch,&text_w,&text_h);																		//check "char" size 
					
							if ((textrect_out.x+text_w)>TEXT_AREA_MAX_X) {																	//if "char" width > (max line width) then calculate next line coordinates
								textrect_out.y+=LINE_Y_STEP;
								textrect_out.x=(ABS_IMG_X + IMG_BORDER);
								textrect_out.w=text_w;
							}

							if ((textrect_out.y+LINE_Y_STEP)>TEXT_AREA_MAX_Y) break;												//if "next line" is outside popup frame then break		

							if (((ch[0]-'0')!=(-0x10)) || ((textrect_out.x)!=(ABS_IMG_X + IMG_BORDER))) {		//Print char on surface and ignore "space" as first char on line
								pop_text = TTF_RenderUTF8_Blended(popup_font, ch, popup_white);
								SDL_BlitSurface(pop_text, &textrect_in, popup_wnd, &textrect_out);
								textrect_out.x+=text_w;
							}
						}
					}

					if (popup_background) {
						imgrect.y=(IMG_H-IMG_BORDER);
						imgrect.h=IMG_BORDER;
						popuprect.y+=LINE_Y_STEP;
						popuprect.h=IMG_BORDER;
						SDL_BlitSurface(popup_background, &imgrect, popup_wnd, &popuprect);
					}

					SDL_FreeSurface(pop_text);
				}
				TTF_CloseFont(popup_font);																														//close font file
			}
			TTF_Quit();																																							//quit SDL_ttf module
		}

		if (popup_background) SDL_FreeSurface(popup_background);																	//free bmp file

		//make popup window transparent
		int x, y, a;																																						
		Uint32 *pixels = (Uint32 *)popup_wnd->pixels;
		Uint32 newPixel;
		for( x = ABS_IMG_X; x < (ABS_IMG_X+IMG_W); x++ ) {
			for (y = ABS_IMG_Y; y < (popuprect.y+imgrect.h); y++) {
				Uint8 r;
				Uint8 g;
				Uint8 b;
				Uint8 a;
				SDL_LockSurface(popup_wnd);
				Uint32 onePixel = pixels[ ( y * popup_wnd->w ) + x ]; 
				SDL_GetRGBA(onePixel, popup_wnd->format, &r, &g, &b, &a);
				if (((y==ABS_IMG_Y) && (r==0x3d) && (g==0x3d) && (b==0x3d)) || (((y==ABS_IMG_Y) || (y==(ABS_IMG_Y+1))) && (r==0x2d) && (g==0x2d) && (b==0x2d))) {
					a = 0;
				} else {
					a = 220;	
				}	
				newPixel = SDL_MapRGBA(popup_wnd->format, r, g, b, a);
				pixels[ ( y * popup_wnd->w ) + x ] = newPixel;   
				SDL_UnlockSurface(popup_wnd);
			}
		}

		SDL_Flip(popup_wnd);																																			//refresh window
		is_write=0;																																								//reset flag "i'm painting"

		while ((p_timeout>0) && (p_timeout<=100) && (is_write==0)) {															//wait declared time or break when other thread painting window
			SDL_Delay(100);
			p_timeout--;
		}
	}

	if ((is_write==0) && (p_timeout==0) && (popup_wnd==SDL_GetVideoSurface())) {								//if any other thread not painting window or timeout not set to unlimitet then:
		if (popuprect.h!=MAX_POPUP_H) popuprect.h = ((popuprect.y+imgrect.h)-ABS_IMG_Y);
		popuprect.y = ABS_IMG_Y;
		SDL_FillRect(popup_wnd, &popuprect, 0);																										//clear popup window
		SDL_Flip(popup_wnd);																																			//refresh window
	}

	//fprintf(pop, "POPUP -> %d - %ld with: FLAG=%d TIME=%d\n", i, (long int)my_id, flag_popup, p_timeout);
	//fflush(pop);
	//fclose(pop);
	
	i--;
	if (i==0) SDL_FreeSurface(popup_wnd);
	return NULL;
}

int show_popup(char *_popup_caption, char *_popup_text, int _popup_timeout) 
{
	if (SDL_GetVideoSurface()==NULL) {                                     											//check VideoSurface, if exists then get it or create new if not
		popup_wnd = SDL_SetVideoMode(POPUP_SCREEN_W, POPUP_SCREEN_H, POPUP_SCREEN_BPP, POPUP_SCREEN_FLAGS);
	} else {
		popup_wnd = SDL_GetVideoSurface();
	}

	if (!popup_wnd) return (-1);

	pthread_t proc_id[i];

	if (_popup_timeout<0) {
		popup_timeout=0;
	} else if (_popup_timeout>10) {
		popup_timeout=11;
	} else {
		popup_timeout=_popup_timeout;
	}
	popup_caption=_popup_caption;
	popup_text=_popup_text;
	pthread_create(&proc_id[i++],NULL,&_show_popup,NULL);

	return (i);
}


