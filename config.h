/* See LICENSE file for copyright and license details. */

/* appearance */
static unsigned int borderpx       = 2;    /* border pixel of windows */
static unsigned int snap           = 32;  /* snap pixel */
static unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static unsigned int systrayonleft  = 0;   /* 0: systray in the right corner, >0: systray on left of status text */
static unsigned int systrayspacing = 2;   /* systray spacing */
static int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static int showsystray             = 1;   /* 0 means no systray */
static int swallowfloating         = 0;   /* 1 means swallow floating windows by default */
static int showbar                 = 1;   /* 0 means no bar */
static int topbar                  = 1;   /* 0 means bottom bar */
static int focusonwheel            = 0;
static char font[128]              = "Hack Nerd Font:bold:size=10";
static const char *fonts[]         = { font };
static char normbgcolor[]          = "#000000";
static char normbordercolor[]      = "#2193ca";
static char normfgcolor[]          = "#2193ca";
static char selfgcolor[]           = "#000000";
static char selbordercolor[]       = "#2193ca";
static char selbgcolor[]           = "#2193ca";
static char *colors[][3] = {
       /*               fg           bg           border   */
       [SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor },
       [SchemeSel]  = { selfgcolor,  selbgcolor,  selbordercolor  },
};

typedef struct {
   const char *name;
   const void *cmd;
} Sp;
const char *spcmd1[] = { "st", "-n", "spterm", "-t", "scratchpad", "-g", "120x34", NULL };
const char *spcmd2[] = { "st", "-n", "spgotop", "-t", "gotop", "-g", "144x34", "-e", "gotop", NULL };
const char *spcmd3[] = { "st", "-n", "splf", "-t", "lf", "-g", "120x34", "-e", "lf", NULL };
static Sp scratchpads[] = {
   /* name          cmd  */
   {"spterm",      spcmd1},
   {"spgotop",     spcmd2},
   {"splf",        spcmd3},
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class            instance   title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Google-chrome",  NULL,      NULL,           1,         0,          0,          -1,        -1 },
	{ "Emacs",          NULL,      NULL,           1 << 1,    0,          0,          -1,        -1 },
	{ "discord",        NULL,      NULL,           1 << 3,    0,          0,          -1,        -1 },
	{ "Spotify",        NULL,      NULL,           1 << 4,    0,          0,          -1,        -1 },
	{ "st",             NULL,      NULL,           0,         0,          1,           0,        -1 },
	{ "Alacritty",      NULL,      NULL,           0,         0,          1,           0,        -1 },
    { NULL,             "spterm",  NULL,           SPTAG(0),  1,          1,           0,        -1 },
	{ NULL,             "spgotop", NULL,           SPTAG(1),  1,          1,           0,        -1 },
	{ NULL,             "splf",    NULL,           SPTAG(2),  1,          1,           0,        -1 },
	{ NULL,             NULL,      "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static float mfact        = 0.50; /* factor of master area size [0.05..0.95] */
static int nmaster        = 1;    /* number of clients in master area */
static int resizehints    = 0;    /* 1 means respect size hints in tiled resizals */
static int lockfullscreen = 0;    /* 1 will force focus on the fullscreen window */

#include "layouts.c"
static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "|M|",      centeredmaster },
	{ ">M>",      centeredfloatingmaster },
	{ "TTT",      bstack },
	{ "===",      bstackhoriz },
	{ "HHH",      grid },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },
#define STACKKEYS(MOD,ACTION) \
	{ MOD, XK_n,     ACTION##stack, {.i = INC(+1) } }, \
	{ MOD, XK_p,     ACTION##stack, {.i = INC(-1) } }, \
	{ MOD, XK_grave, ACTION##stack, {.i = PREVSEL } }, \
	{ MOD, XK_a,     ACTION##stack, {.i = 0 } }, \
	{ MOD, XK_2,     ACTION##stack, {.i = 1 } }, \
	{ MOD, XK_3,     ACTION##stack, {.i = 2 } }, \
	{ MOD, XK_e,     ACTION##stack, {.i = -1 } },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static char hpapps[256] = "google-chrome-stable,emacs,discord,spotify";
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-hp", hpapps, NULL };
static const char *termcmd[]  = { "st", NULL };

/*
 * Xresources preferences to load at startup
 */
ResourcePref resources[] = {
		{ "font",                    STRING,  &font },
		{ "color0",                  STRING,  &normbordercolor },
		{ "color6",                  STRING,  &selbordercolor },
		{ "color0",                  STRING,  &normbgcolor },
		{ "color6",                  STRING,  &normfgcolor },
		{ "color0",                  STRING,  &selfgcolor },
		{ "color6",                  STRING,  &selbgcolor },
		{ "hpapps",                  STRING,  &hpapps },
		{ "borderpx",          	     INTEGER, &borderpx },
		{ "snap",          		     INTEGER, &snap },
		{ "showbar",          	     INTEGER, &showbar },
		{ "topbar",          	     INTEGER, &topbar },
		{ "nmaster",          	     INTEGER, &nmaster },
		{ "resizehints",       	     INTEGER, &resizehints },
		{ "lockfullscreen",          INTEGER, &lockfullscreen },
		{ "mfact",      	 	     FLOAT,   &mfact },
		{ "systraypinning",          INTEGER, &systraypinning },
		{ "systrayonleft",           INTEGER, &systrayonleft },
		{ "systrayspacing",          INTEGER, &systrayspacing },
		{ "systraypinningfailfirst", INTEGER, &systraypinningfailfirst },
		{ "showsystray",             INTEGER, &showsystray },
		{ "swallowfloating",         INTEGER, &swallowfloating },
};


static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_d,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_Return, togglescratch,  {.ui = 0 } },
	{ MODKEY,                       XK_1,      togglescratch,  {.ui = 1 } },
	{ MODKEY,                       XK_semicolon, togglescratch,  {.ui = 2 } },
	{ MODKEY|ShiftMask,             XK_b,      togglebar,      {0} },
	STACKKEYS(MODKEY,                          focus)
	STACKKEYS(MODKEY|ShiftMask,                push)
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_o,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_b,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_f,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY|ShiftMask,             XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_u,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY|ShiftMask,             XK_u,      setlayout,      {.v = &layouts[4]} },
	{ MODKEY,                       XK_y,      setlayout,      {.v = &layouts[5]} },
	{ MODKEY|ShiftMask,             XK_y,      setlayout,      {.v = &layouts[6]} },
	{ MODKEY,                       XK_g,      setlayout,      {.v = &layouts[7]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_Left,   focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_Right,  focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_Left,   tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_Right,  tagmon,         {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  spawn,          SHCMD("playerctl -p spotify previous") },
	{ MODKEY|ShiftMask,             XK_period, spawn,          SHCMD("playerctl -p spotify next") },
	{ MODKEY,                       XK_period, spawn,          SHCMD("playerctl -p spotify play-pause") },
	{ MODKEY,                       XK_minus,  spawn,          SHCMD("pamixer --allow-boost -d 25; kill -44 $(pidof dwmblocks)") },
	{ MODKEY,                       XK_equal,  spawn,          SHCMD("pamixer --allow-boost -i 25; kill -44 $(pidof dwmblocks)") },
	{ MODKEY|ShiftMask,             XK_minus,  spawn,          SHCMD("pamixer -t; kill -44 $(pidof dwmblocks)") },
	{ MODKEY|ShiftMask,             XK_equal,  spawn,          SHCMD("pamixer --set-volume 100; kill -44 $(pidof dwmblocks)") },
	{ MODKEY|ShiftMask,             XK_s,      spawn,          SHCMD("flameshot gui") },
	{ MODKEY|ControlMask,           XK_s,      spawn,          SHCMD("flameshot full") },
	TAGKEYS(                        XK_z,                      0)
	TAGKEYS(                        XK_x,                      1)
	TAGKEYS(                        XK_c,                      2)
	TAGKEYS(                        XK_v,                      3)
	TAGKEYS(                        XK_j,                      4)
	TAGKEYS(                        XK_k,                      5)
	TAGKEYS(                        XK_l,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_r,      quit,           {0} },
	{ MODKEY|ShiftMask,             XK_q,      spawn,          SHCMD("pwr") },
	{ ControlMask,                  XK_space,  spawn,          SHCMD("dunstctl close") },
	{ ControlMask|ShiftMask,        XK_space,  spawn,          SHCMD("dunstctl close-all") },
	{ ControlMask,                  XK_grave,  spawn,          SHCMD("dunstctl history-pop") }
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

