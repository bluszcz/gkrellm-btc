
#if !defined(WIN32)
#include <gkrellm2/gkrellm.h>
#else
#include <src/gkrellm.h>
#include <src/win32-plugin.h>
#endif

#define	CONFIG_NAME	"BTC-E Buy Chart"
#define	STYLE_NAME	"btce"

static GkrellmMonitor *monitor;
static GkrellmPanel   *panel;

static GkrellmDecal *text1_btc,
                    *text_ltc_decal,
                    *text_ftc_decal,
                    *pixmap_decal;

static gchar    *scroll_text = "BTC-C Exchange list";
static gchar    *btc_text = "BTC-E.com exchange rates";
static gint     style_id;

#if defined(GKRELLM_HAVE_DECAL_SCROLL_TEXT)
static gboolean		scroll_loop_mode;
#endif

static gint
panel_expose_event(GtkWidget *widget, GdkEventExpose *ev)
	{
	gdk_draw_pixmap(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			panel->pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
			ev->area.width, ev->area.height);
	return FALSE;
	}



static FILE *command_pipe;


const gchar* getfield(char* line, int num)
{
    gchar* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

static void
run_command ()
{
    if (command_pipe)
      		return;
    command_pipe = popen("readbtce.py ftc ltc", "r");
    if (command_pipe)
        fcntl(fileno(command_pipe), F_SETFL, O_NONBLOCK);
}

static gboolean
command_done ()
{
    gchar buf[18];
    while (fread(buf, 1, sizeof(buf) - 1, command_pipe) > 0)
        ;
    if (feof(command_pipe))
    {
    	buf[strlen(buf)-1] = 0;
    	buf[strlen(buf)-1] = 0;
    	char* tmp = strdup(buf);
		scroll_text = &buf[0] ;
		scroll_text =  getfield(tmp, 1);
		gkrellm_decal_scroll_text_set_text(panel, text_ftc_decal, scroll_text);
		tmp = strdup(buf);
		scroll_text =  getfield(tmp, 2);
		gkrellm_decal_scroll_text_set_text(panel, text_ltc_decal, scroll_text);
        pclose(command_pipe);
        command_pipe = NULL;
        return TRUE;
    }
    return FALSE;
}


static void
update_plugin()
	{
	gint		w_scroll, w_decal;
	static gint	x_scroll;

    if (command_pipe)
    {
    	command_done();

    }
	 if(GK.timer_ticks % 6 == 0)
    {
        run_command();
    }

#if defined(GKRELLM_HAVE_DECAL_SCROLL_TEXT)
	gkrellm_decal_scroll_text_set_text(panel, text1_btc, btc_text);
	gkrellm_decal_scroll_text_get_size(text1_btc, &w_scroll, NULL);
	gkrellm_decal_get_size(text1_btc, &w_decal, NULL);
	x_scroll -= 1;
	if (x_scroll <= -w_scroll)
		x_scroll = scroll_loop_mode ? 0 : w_decal;
	gkrellm_decal_text_set_offset(text1_btc, x_scroll, 0);
#else
	w_decal = text1_btc->w;
	w_scroll = gdk_string_width(text1_btc->text_style.font, scroll_text);
	x_scroll -= 1;
	if (x_scroll <= -w_scroll)
		x_scroll = w_decal;
	text1_btc->x_off = x_scroll;
	gkrellm_draw_decal_text(panel, text1_btc, scroll_text, x_scroll);
#endif
	gkrellm_draw_panel_layers(panel);
	}

static void
create_plugin(GtkWidget *vbox, gint first_create)
	{
	GkrellmStyle	*style;
	GkrellmTextstyle *ts, *ts_alt;
	GdkPixmap		*pixmap;
	GdkBitmap		*mask;
	gint			y;
	gint			x;


	if (first_create)
		panel = gkrellm_panel_new0();

	style = gkrellm_meter_style(style_id);
	ts = gkrellm_meter_textstyle(style_id);
	ts_alt = gkrellm_meter_alt_textstyle(style_id);
	text1_btc = gkrellm_create_decal_text(panel, "Ay", ts, style,
				-1,     /* x = -1 places at left margin */
				-1,     /* y = -1 places at top margin	*/
				-1);    /* w = -1 makes decal the panel width minus margins */
	text_ltc_decal = gkrellm_create_decal_text(panel, "Ay", ts, style,
				-1,     /* x = -1 places at left margin */
				text1_btc->y + text1_btc->h + 2,     /* y = -1 places at top margin	*/
				-1);    /* w = -1 makes decal the panel width minus margins */
	text_ftc_decal = gkrellm_create_decal_text(panel, "Ay", ts, style,
				-1,     /* x = -1 places at left margin */
				text_ltc_decal->y + text_ltc_decal->h + 2,     /* y = -1 places at top margin	*/
				-1);    /* w = -1 makes decal the panel width minus margins */

	gkrellm_panel_configure(panel, NULL, style);
	gkrellm_panel_create(vbox, monitor, panel);


	}


static GkrellmMonitor	plugin_mon	=
	{
	CONFIG_NAME,        /* Name, for config tab.    */
	0,                  /* Id,  0 if a plugin       */
	create_plugin,      /* The create function      */
	update_plugin,      /* The update function      */
	NULL,               /* The config tab create function   */
	NULL,               /* Apply the config function        */

	NULL,               /* Save user config   */
	NULL,               /* Load user config   */
	NULL,               /* config keyword     */

	NULL,               /* Undefined 2  */
	NULL,               /* Undefined 1  */
	NULL,               /* private      */

	MON_MAIL,           /* Insert plugin before this monitor */

	NULL,               /* Handle if a plugin, filled in by GKrellM     */
	NULL                /* path if a plugin, filled in by GKrellM       */
	};

#if defined(WIN32)
__declspec(dllexport) GkrellmMonitor *
gkrellm_init_plugin(win32_plugin_callbacks* calls)
#else
GkrellmMonitor *
gkrellm_init_plugin(void)
#endif
{
	style_id = gkrellm_add_meter_style(&plugin_mon, STYLE_NAME);
	monitor = &plugin_mon;
	return &plugin_mon;
}
