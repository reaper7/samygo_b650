#define CONF_FILE	"netremote.conf"
#define NETREMOTE_LOG	"/mtd_ram/netremote.log"

static char path[256];

int max_modes=1;
int remote_mode=1;
int power_mode=0;

FILE *fl;
const char *logfile = NETREMOTE_LOG;

char *remote_mode_text[]={"ORYGINAL SAMSUNG MODE","HALF SAT MODE","FULL SAT MODE"};

