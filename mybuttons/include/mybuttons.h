#define SCREEN_W	(1920/2)	// screen surface resolution: x
#define SCREEN_H	(1080/2)	// screen surface resolution: y
#define SCREEN_BPP	32		// screen bits-per-pixel value
#define SCREEN_FLAGS	SDL_HWSURFACE	// screen flags now is SDL_HWSURFACE ; (0 or maybe SDL_SWSURFACE ?)
#define FONT_SIZE	18
#define TEXT_SIZE	195
#define SC_BUTTONSBMP	"sc_buttons.png"
#define SC_FONT		"/mtd_appdata/Font/shadow.ttf"
#define CONF_FILE	"mybuttons.conf"
#define LEFT_ABS 5
#define TOP_ABS (SCREEN_H - 45)
#define CAPTION_STEP	228
#define CAPTION_LEFT_ABS (LEFT_ABS+45)
#define CAPTION_TOP_ABS (TOP_ABS+12)
	
#define	RED	0
#define	GREEN	1
#define	YELLOW	2
#define	BLUE	3

#define MYBUTTONS_LOG	"/mtd_ram/mybuttons.log"

#define CMDMAX 100
#define DEF_SLEEP 500000

static char path[256];

SDL_Surface *screen;
SDL_Surface *rtext_fg, *gtext_fg, *ytext_fg, *btext_fg, *sc_buttons;
SDL_Surface *rtext_bg, *gtext_bg, *ytext_bg, *btext_bg;
SDL_Rect dstrect = {LEFT_ABS, TOP_ABS, SCREEN_W-(LEFT_ABS*2), 40};

SDL_Rect rtdrect = {CAPTION_LEFT_ABS, CAPTION_TOP_ABS, TEXT_SIZE, FONT_SIZE+2};
SDL_Rect gtdrect = {CAPTION_LEFT_ABS+CAPTION_STEP, CAPTION_TOP_ABS, TEXT_SIZE, FONT_SIZE+2};
SDL_Rect ytdrect = {CAPTION_LEFT_ABS+(CAPTION_STEP*2), CAPTION_TOP_ABS, TEXT_SIZE, FONT_SIZE+2};
SDL_Rect btdrect = {CAPTION_LEFT_ABS+(CAPTION_STEP*3), CAPTION_TOP_ABS, TEXT_SIZE, FONT_SIZE+2};

SDL_Rect rtsrect_fg = {0,0,TEXT_SIZE,FONT_SIZE+2};
SDL_Rect gtsrect_fg = {0,0,TEXT_SIZE,FONT_SIZE+2};
SDL_Rect ytsrect_fg = {0,0,TEXT_SIZE,FONT_SIZE+2};
SDL_Rect btsrect_fg = {0,0,TEXT_SIZE,FONT_SIZE+2};

TTF_Font *font;
//SDL_Color black = {1,1,1};
SDL_Color white = {255,255,255};

int flag_screen=0;
int flag_font=0;
int flag_macro=0;

struct datas
{
  char bcaption[50];
  char bcommand[255];
}my_config [50];

int posmax=0;
int poscur;

int start_button=0;
int pause_button=0;
int popup_msg=0;

FILE *fl;
const char *logfile = MYBUTTONS_LOG;

char *rc_codes[]={"KEY_SAP", "KEY_SOURCE", "KEY_POWER", "KEY_SLEEP", "KEY_NUM1", "KEY_NUM2", "KEY_NUM3", "KEY_VOL_PLUS", "KEY_NUM4", "KEY_NUM5", "KEY_NUM6", "KEY_VOL_MINUS", "KEY_NUM7", "KEY_NUM8", "KEY_NUM9", "KEY_MUTE", "KEY_P_DOWN", "KEY_NUM0", "KEY_P_UP", "KEY_PRE_CH", "KEY_COL_GREEN", "KEY_COL_YELLOW", "KEY_COL_BLUE", "\0", "\0", "\0", "KEY_MENU", "KEY_TV", "\0", "\0", "KEY_INFO", "\0", "KEY_PIP", "\0", "\0", "\0", "\0", "KEY_SUBT", "\0", "\0", "KEY_PICMODE", "\0", "\0", "KEY_SOUND_MODE", "KEY_TTX_MIX", "KEY_EXIT", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "KEY_AIR_CABLE", "\0", "\0", "\0", "\0", "KEY_SERVICE", "\0", "\0", "KEY_ASPECT", "\0", "\0", "\0", "\0", "KEY_DTV", "KEY_FAV_CH", "KEY_REWIND", "KEY_STOP", "KEY_PLAY", "KEY_FORWARD", "KEY_RECORD", "KEY_PAUSE", "KEY_TOOLS", "\0", "\0", "\0", "KEY_GUIDE", "\0", "\0", "\0", "KEY_INTERNET2", "\0", "\0", "\0", "\0", "KEY_RETURN", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "KEY_CUR_UP", "KEY_CUR_DOWN", "KEY_CUR_RIGHT", "\0", "\0", "KEY_CUR_LEFT", "\0", "\0", "KEY_ENTER", "KEY_VGA", "\0", "KEY_CH_LIST", "KEY_COL_RED", "\0", "KEY_SRS", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "KEY_ESAVING", "\0", "KEY_CONTENT", "KEY_DISP_VCR", "KEY_DISP_CABLE", "\0", "KEY_DISP_TV", "KEY_DISP_DVD", "KEY_DISP_STB", "\0", "\0", "\0", "\0", "KEY_EXT1", "\0", "KEY_COMPONENT", "\0", "\0", "\0", "\0", "KEY_HDMI_TOGGLE", "KEY_MEDIA_P", "\0", "\0", "\0", "\0", "\0", "\0", "KEY_INTERNET", "\0", "\0", "\0", "\0", "KEY_OFF", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "KEY_PIC_DYNAMIC", "KEY_HDMI2", "\0", "KEY_DIAG1", "KEY_DIAG2", "KEY_HDMI3", "\0", "\0", "KEY_HDMI4", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "KEY_PIC_MOVIE", "KEY_PIC_STANDARD", "\0", "\0", "\0", "KEY_ASPECT43", "KEY_ASPECT169", "\0", "\0", "\0", "\0", "KEY_HDMI1", "\0", "KEY_EXT2", "KEY_CVBS", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "KEY_NOTHING", "\0", "KEY_BEZEL_SOURCE", "KEY_BEZEL_POWER", "\0", "\0", "\0", "\0", "KEY_BEZEL_VOL_UP", "\0", "\0", "\0", "KEY_BEZEL_VOL_DOWN", "\0", "\0", "\0", "\0", "KEY_BEZEL_P_DOWN", "\0", "KEY_BEZEL_P_UP", "END"};

