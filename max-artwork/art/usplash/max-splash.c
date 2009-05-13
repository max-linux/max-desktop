/* usplash
 *
 * eft-theme.c - definition of eft theme
 *
 * Copyright © 2006 Dennis Kaarsemaker <dennis@kaarsemaker.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */
/*
#include <stdio.h>
#include <string.h>
*/

#include <usplash-theme.h>
/* Needed for the custom drawing functions */
#include <usplash_backend.h>
extern struct usplash_pixmap pixmap_usplash_640_480, pixmap_usplash_800_600, pixmap_usplash_1024_768;
extern struct usplash_pixmap pixmap_throbber_back;
extern struct usplash_pixmap pixmap_throbber_back_16;
extern struct usplash_pixmap pixmap_throbber_back_32;
extern struct usplash_pixmap pixmap_throbber_fore;
extern struct usplash_pixmap pixmap_throbber_fore_16;
extern struct usplash_pixmap pixmap_throbber_fore_32;

void t_init(struct usplash_theme* theme);
void t_clear_progressbar(struct usplash_theme* theme);
void t_clear_progressbar_16(struct usplash_theme* theme);
void t_clear_progressbar_32(struct usplash_theme* theme);
void t_draw_progressbar(struct usplash_theme* theme, int percentage);
void t_draw_progressbar_16(struct usplash_theme* theme, int percentage);
void t_draw_progressbar_32(struct usplash_theme* theme, int percentage);
void t_animate_step(struct usplash_theme* theme, int pulsating);
void t_animate_step_16(struct usplash_theme* theme, int pulsating);
void t_animate_step_32(struct usplash_theme* theme, int pulsating);

struct usplash_theme usplash_theme;
struct usplash_theme usplash_theme_640_480;
struct usplash_theme usplash_theme_800_600;
struct usplash_theme usplash_theme_1024_768;


/*
short getColorIndex(struct usplash_theme *theme, char *color){
    short i=0, index=0;
    char outStr[256];
    FILE *fp;
    fp = fopen("/tmp/usplash.debug", "a");
    for (i=0; i<=255; i++) {
	    sprintf(outStr,"%02x%02x%02x",theme->pixmap->palette[i][0], theme->pixmap->palette[i][1], theme->pixmap->palette[i][2]);
	    if ( strcmp(color, outStr) == 0) {
               fprintf(fp, "FOUND pixmap %dx%d INDEX=%d in hex is %s\n", theme->theme_width, theme->theme_height, i, outStr);
               index=i;
            }
    }
    fclose(fp);
    return index;
}
*/

//#define BGCOLOR 27,52,56,58,**59**, 89, *91* /* #1675ce */
/* rojo 129, 160, 192 */
#define BGCOLOR 59 /* #1675ce */
#define TEXTFG 0
#define PROBG 129
#define PROFG 0

/* Theme definition */
struct usplash_theme usplash_theme = {
	.version = THEME_VERSION, /* ALWAYS set this to THEME_VERSION, 
                                 it's a compatibility check */
    .next = &usplash_theme_800_600,
    .ratio = USPLASH_4_3,

	/* Background and font */
	.pixmap = &pixmap_usplash_640_480,

        /* theme resolution; if 0, use width/height of pixmap */
        .theme_width = 640,
        .theme_height = 480,

        /* position of pixmap */
        .pixmap_x = 0,
        .pixmap_y = 0,

	/* Palette indexes */
	.background             = BGCOLOR,
  	.progressbar_background = PROBG,
  	.progressbar_foreground = PROFG,
	.text_background        = BGCOLOR,
	.text_foreground        = TEXTFG,
	.text_success           = 251,
	.text_failure           = 106,

	/* Progress bar position and size in pixels */
  	.progressbar_x      = 198,
  	.progressbar_y      = 233,
  	.progressbar_width  = 244,
  	.progressbar_height = 3,

	/* Text box position and size in pixels */
  	.text_x      = 120,
  	.text_y      = 307,
  	.text_width  = 360,
  	.text_height = 100,

	/* Text details */
  	.line_height  = 15,
  	.line_length  = 32,
  	.status_width = 35,

    /* Functions */
    .init = t_init,
    .clear_progressbar = t_clear_progressbar_16,
    .draw_progressbar = t_draw_progressbar_16,
    .animate_step = t_animate_step_16,
};


struct usplash_theme usplash_theme_800_600 = {
	.version = THEME_VERSION, /* ALWAYS set this to THEME_VERSION, 
                                 it's a compatibility check */
    .next = &usplash_theme_1024_768,
    .ratio = USPLASH_4_3,

	/* Background and font */
	.pixmap = &pixmap_usplash_800_600,

        /* theme resolution; if 0, use width/height of pixmap */
        .theme_width = 800,
        .theme_height = 600,

        /* position of pixmap */
        .pixmap_x = 0,
        .pixmap_y = 0,

	/* Palette indexes */
	.background             = BGCOLOR,
  	.progressbar_background = PROBG,
  	.progressbar_foreground = PROFG,
	.text_background        = BGCOLOR,
	.text_foreground        = TEXTFG,
	.text_success           = 251,
	.text_failure           = 106,

	/* Progress bar position and size in pixels */
  	.progressbar_x      = 278,
  	.progressbar_y      = 292,
  	.progressbar_width  = 244,
  	.progressbar_height = 3,

	/* Text box position and size in pixels */
  	.text_x      = 220,
  	.text_y      = 407,
  	.text_width  = 360,
  	.text_height = 150,

	/* Text details */
  	.line_height  = 15,
  	.line_length  = 32,
  	.status_width = 35,

    /* Functions */
    .init = t_init,
    .clear_progressbar = t_clear_progressbar_16,
    .draw_progressbar = t_draw_progressbar_16,
    .animate_step = t_animate_step_16,
};


struct usplash_theme usplash_theme_1024_768 = {
	.version = THEME_VERSION,
    .next = NULL,
    .ratio = USPLASH_4_3,

	/* Background and font */
	.pixmap = &pixmap_usplash_1024_768,

        /* theme resolution; if 0, use width/height of pixmap */
        .theme_width = 1024,
        .theme_height = 768,

        /* position of pixmap */
        .pixmap_x = 0,
        .pixmap_y = 0,

	/* Palette indexes */
	.background             = BGCOLOR,
  	.progressbar_background = PROBG,
  	.progressbar_foreground = PROFG,
	.text_background        = BGCOLOR,
	.text_foreground        = TEXTFG,
	.text_success           = 251,
	.text_failure           = 106,

	/* Progress bar position and size in pixels */
  	.progressbar_x      = 358,
  	.progressbar_y      = 373,
  	.progressbar_width  = 308,
  	.progressbar_height = 3,

	/* Text box position and size in pixels */
  	.text_x      = 322,
  	.text_y      = 475,
  	.text_width  = 380,
  	.text_height = 200,

	/* Text details */
  	.line_height  = 15,
  	.line_length  = 32,
  	.status_width = 35,

    /* Functions */
    .init = t_init,
    .clear_progressbar = t_clear_progressbar,
    .draw_progressbar = t_draw_progressbar,
    .animate_step = t_animate_step,
};


void t_init(struct usplash_theme *theme) {
    int x, y;
    usplash_getdimensions(&x, &y);
    theme->progressbar_x = (x - usplash_theme_width(theme))/2 + theme->progressbar_x;
    theme->progressbar_y = (y - usplash_theme_height(theme))/2 + theme->progressbar_y;
    /*
    printf("background=%d text_background=%d\n", theme->background, theme->text_background);
    theme->background=getColorIndex(theme, "1675ce");
    theme->text_background=getColorIndex(theme, "1675ce");
    printf("background=%d text_background=%d\n", theme->background, theme->text_background);
    */
}

void t_clear_progressbar(struct usplash_theme *theme) {
    usplash_put(theme->progressbar_x, theme->progressbar_y, &pixmap_throbber_back);
}

void t_clear_progressbar_16(struct usplash_theme *theme) {
    usplash_put(theme->progressbar_x, theme->progressbar_y, &pixmap_throbber_back_16);
}

void t_clear_progressbar_32(struct usplash_theme *theme) {
    usplash_put(theme->progressbar_x, theme->progressbar_y, &pixmap_throbber_back_32);
}

void t_draw_progressbar(struct usplash_theme *theme, int percentage) {
    int w = (pixmap_throbber_back.width * percentage / 100);
    if(percentage == 0)
        usplash_put(theme->progressbar_x, theme->progressbar_y, &pixmap_throbber_back);
    if(percentage < 0){/* Unloading */
        w *= -1;
        /* Draw background to left of foreground */
        usplash_put_part(theme->progressbar_x, theme->progressbar_y, w, pixmap_throbber_back.height, 
                         &pixmap_throbber_back, 0, 0);
        /* Draw foreground to right of background */
        usplash_put_part(theme->progressbar_x + w, theme->progressbar_y, pixmap_throbber_back.width - w,
                         pixmap_throbber_back.height, &pixmap_throbber_fore, w, 0);
    }
    else{/* Loading */
        /* Draw foreground to left of background */
        usplash_put_part(theme->progressbar_x, theme->progressbar_y, w, pixmap_throbber_back.height, 
                         &pixmap_throbber_fore, 0, 0);
        /* Draw background ot right of foreground */
        usplash_put_part(theme->progressbar_x + w, theme->progressbar_y, pixmap_throbber_back.width - w, pixmap_throbber_back.height, 
                         &pixmap_throbber_back, w, 0);
    }
}

void t_draw_progressbar_16(struct usplash_theme *theme, int percentage) {
    int w = (pixmap_throbber_back_16.width * percentage / 100);
    if (percentage == 0)
        usplash_put(theme->progressbar_x, theme->progressbar_y, &pixmap_throbber_back_16);
    if (percentage < 0){ /* Unloading */
        w *= -1;
        /* Draw background to left of foreground */
        usplash_put_part(theme->progressbar_x, theme->progressbar_y, w, pixmap_throbber_back_16.height, 
                         &pixmap_throbber_back_16, 0, 0);
        /* Draw foreground to right of background */
        usplash_put_part(theme->progressbar_x + w, theme->progressbar_y, pixmap_throbber_back_16.width - w,
                         pixmap_throbber_back_16.height, &pixmap_throbber_fore_16, w, 0);
    }
    else{/* Loading */
        /* Draw foreground to left of background */
        usplash_put_part(theme->progressbar_x, theme->progressbar_y, w, pixmap_throbber_back_16.height, 
                         &pixmap_throbber_fore_16, 0, 0);
        /* Draw background to right of foreground */
        usplash_put_part(theme->progressbar_x + w, theme->progressbar_y, pixmap_throbber_back_16.width - w, pixmap_throbber_back_16.height, 
                         &pixmap_throbber_back_16, w, 0);
    }
}

void t_draw_progressbar_32(struct usplash_theme *theme, int percentage) {
    int w = (pixmap_throbber_back_32.width * percentage / 100);
    if (percentage == 0)
        usplash_put(theme->progressbar_x, theme->progressbar_y, &pixmap_throbber_back_32);
    if (percentage < 0){ /* Unloading */
        w *= -1;
        /* Draw background to left of foreground */
        usplash_put_part(theme->progressbar_x, theme->progressbar_y, w, pixmap_throbber_back_32.height, 
                         &pixmap_throbber_back_32, 0, 0);
        /* Draw foreground to right of background */
        usplash_put_part(theme->progressbar_x + w, theme->progressbar_y, pixmap_throbber_back_32.width - w,
                         pixmap_throbber_back_32.height, &pixmap_throbber_fore_32, w, 0);
    }
    else{/* Loading */
        /* Draw foreground to left of background */
        usplash_put_part(theme->progressbar_x, theme->progressbar_y, w, pixmap_throbber_back_32.height, 
                         &pixmap_throbber_fore_32, 0, 0);
        /* Draw background to right of foreground */
        usplash_put_part(theme->progressbar_x + w, theme->progressbar_y, pixmap_throbber_back_32.width - w, pixmap_throbber_back_32.height, 
                         &pixmap_throbber_back_32, w, 0);
    }
}


void t_animate_step(struct usplash_theme* theme, int pulsating) {

    static int pulsate_step = 0;
    static int pulse_width = 16;
    static int step_width = 2;
    static int num_steps = 0;
    int x1;
    int x2;
    num_steps = (pixmap_throbber_fore.width - pulse_width)/2;

    if (pulsating) {
        if(pulsate_step < num_steps/2+1){
	        x1 = 2 * step_width * pulsate_step;
        }
        else{
	        x1 = pixmap_throbber_fore.width - pulse_width - 2 * step_width * (pulsate_step - num_steps/2+1);
        }
        x2 = x1 + pulse_width;

        /* Draw progress bar background on left side of foreground 'pulse' */
        usplash_put_part(theme->progressbar_x, theme->progressbar_y, x1,
                         pixmap_throbber_back.height, &pixmap_throbber_back, 0, 0);
        /* Draw progress bar foreground 'pulse' */
        usplash_put_part(theme->progressbar_x + x1, theme->progressbar_y, pulse_width,
                         pixmap_throbber_back.height, &pixmap_throbber_fore, x1, 0);
        /* Draw progress bar background on right side of foreground 'pulse' */
        usplash_put_part(theme->progressbar_x + x2, theme->progressbar_y, pixmap_throbber_back.width - x2,
                         pixmap_throbber_back.height, &pixmap_throbber_back, x2, 0);

        pulsate_step = (pulsate_step + 1) % num_steps;
    }
}

void t_animate_step_16(struct usplash_theme* theme, int pulsating) {

    static int pulsate_step = 0;
    static int pulse_width = 8;
    static int step_width = 2;
    static int num_steps = 0;
    int x1;
    int x2;
    num_steps = (pixmap_throbber_fore_16.width - pulse_width)/2;

    if (pulsating) {
        if(pulsate_step < num_steps/2+1){
	        x1 = 2 * step_width * pulsate_step;
        }
        else{
            x1 = pixmap_throbber_fore_16.width - pulse_width - 2 * step_width * (pulsate_step - num_steps/2+1);
        }
        x2 = x1 + pulse_width;

        /* Draw progress bar background on left side of foreground 'pulse' */
        usplash_put_part(theme->progressbar_x, theme->progressbar_y, x1,
                         pixmap_throbber_back_16.height, &pixmap_throbber_back_16, 0, 0);
        /* Draw progress bar foreground 'pulse' */
        usplash_put_part(theme->progressbar_x + x1, theme->progressbar_y, pulse_width,
                         pixmap_throbber_back_16.height, &pixmap_throbber_fore_16, x1, 0);
        /* Draw progress bar background on right side of foreground 'pulse' */
        usplash_put_part(theme->progressbar_x + x2, theme->progressbar_y, pixmap_throbber_back_16.width - x2,
                         pixmap_throbber_back_16.height, &pixmap_throbber_back_16, x2, 0);

        pulsate_step = (pulsate_step + 1) % num_steps;
    }
}

void t_animate_step_32(struct usplash_theme* theme, int pulsating) {

    static int pulsate_step = 0;
    static int pulse_width = 32;
    static int step_width = 2;
    static int num_steps = 0;
    int x1;
    int x2;
    num_steps = (pixmap_throbber_fore_32.width - pulse_width)/2;

    if (pulsating) {
        if(pulsate_step < num_steps/2+1){
	        x1 = 2 * step_width * pulsate_step;
        }
        else{
            x1 = pixmap_throbber_fore_32.width - pulse_width - 2 * step_width * (pulsate_step - num_steps/2+1);
        }
        x2 = x1 + pulse_width;

        /* Draw progress bar background on left side of foreground 'pulse' */
        usplash_put_part(theme->progressbar_x, theme->progressbar_y, x1,
                         pixmap_throbber_back_32.height, &pixmap_throbber_back_32, 0, 0);
        /* Draw progress bar foreground 'pulse' */
        usplash_put_part(theme->progressbar_x + x1, theme->progressbar_y, pulse_width,
                         pixmap_throbber_back_32.height, &pixmap_throbber_fore_32, x1, 0);
        /* Draw progress bar background on right side of foreground 'pulse' */
        usplash_put_part(theme->progressbar_x + x2, theme->progressbar_y, pixmap_throbber_back_32.width - x2,
                         pixmap_throbber_back_32.height, &pixmap_throbber_back_32, x2, 0);

        pulsate_step = (pulsate_step + 1) % num_steps;
    }
}
