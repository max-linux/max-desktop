This theme is released under the GNU General Public License
***********************************************************
Name = Kubuntu Ultra Splash (Moodin)
Description = Sleek & Professional KSplash Theme for Kubuntu
Version = 1.5.1
Author = Jim Bustos <jimbustos@gmail.com>
URL = http://www.kde-look.org/usermanager/search.php?username=jbus

This theme also has a matching KDM theme located @
http://kdelook.org/content/show.php?content=29331

This theme requires Moodwrod's Moodin Engine.
The moodin engine is/will be included in Dapper and is also available for Breezy in backports.

This theme includes the official Kubuntu Breezy Wallpaper. All credit for this wallpaper goes to the author.

This theme works with most common screen resolutions. In addition, it has backgrounds included for wide-screen and high-resolution monitors.


Changelog:
**********
1.5.1
Tweaked logo

1.5
Changed License to GPL
Optimized Theme for latest version of Moodin Engine and for a more consistent look.
Added the Moodin Engine's Translate option for translation of 'Welcome' to different languages.
Changed greeting to 'Welcome <fullusername>' instead of 'Welcome <loginname>'.
Changed background to official Kubuntu Breezy wallpaper.
Updated icon choices, font size, and colors for a cleaner look.
***Included 1600x1200 backgrounds as well as 1900x1200, 1440x900 & 1280x800 backgrounds for widescreen users.
Updated installation instructions.


Installation:
*************
Use the Splash Screen Theme Manager in 'System Settings'->'Desktop'->'Splash Screen' to install this theme.
Be sure that your Login manager background is set to use the official Kubuntu wallpaper for consistency. This can be set in 'System Settings'->'Login Manager'->'Background'.

Using your own background:
**************************
1.Extract kubuntuUltraSplash.tar.gz
2.Open up your wallpaper in the Gimp and open the Dialog.png file included the "Source Images" folder as a new layer. 
3.Export the resulting image back to the KubuntuUltraSplash folder and be sure to name it Background.jpg.
4.Compress the folder as KubuntuUltraSplash.tar.gz and install it as normal.
5.Also be sure to change the 'System Settings'->'Login Manager'->'Background' to the same background for consistency.


***Users of 1900x1200, 1600x1200 & 1440x900. 
********************************************
If you have problems with your background being the wrong size, scaling incorrectly or fonts being too small or too big, do the following:

1.Extract the KubuntuUltraSplash.tar.gz
2.Rename your resolution specific background(i.e Background-1600x1200.jpg) to Background.jpg
3.In Theme.rc change 'BaseResolution =' to your monitors resolution. (This will prevent auto-scaling of background and fonts)
4.Compress the folder as KubuntuUltraSplash.tar.gz and re-install it.
5.Test the theme. If Label and Status Font sizes still need to be adjusted, extract and open Theme.rc, make changes to font sizes, compress folder & re-install.