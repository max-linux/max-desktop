% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Main part.
%
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

% max command line length
/cmdlinelength 512 def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Install source is 64bit?
%
% ( ) ==> ( true|false )
%
/64bit_source {
  % 64 bit dir exists and is != 32 bit dir
  64bit_boot_dir 32bit_boot_dir ne
  64bit_boot_dir .undef ne and
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Are we a dvd?
%
% ( ) ==> ( true|false )
%
/is_dvd {
  % check only once
  is_dvd.result .undef ne { is_dvd.result return } if

  /is_dvd.result
    mediatype m_cdrom eq {
      0x10 readsector
      dup 0x50 add getdword exch free
      9 shr % iso size in MB
      720 gt % assume dvd if > 720 MB
    } {
      false
    } ifelse
  def

  is_dvd.result return
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Are we a live image?
%
% ( ) ==> ( true|false )
%
/is_live {
  % check only once
  is_live.result .undef ne { is_live.result return } if

  /is_live.result "/casper/filesystem.manifest" filesize .undef ne def

  is_live.result return
} def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Install source is 32 & 64bit?
%
% ( ) ==> ( true|false )
%
/32+64bit_source {
  32bit_boot_dir .undef ne
  % uncomment next line to automatically warn about 32bit software on 64bit machines, too
  % 64bit_boot_dir .undef ne and
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Enough memory?
%
% ( ) ==> ( true|false )
%
/enough_mem {
  biosmem 4 20 shl lt
  biosmem 200 20 shl ge or
  syslinux not or
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Check if there are boot directories for 32 & 64bit.
%
% Assumes 32bit to be in *i386* and 64bit in *x86_64*.
%
% ( ) ==> ( )
%
/check_arch_boot_dir {
  getcwd dup .undef ne {
    /64bit.tmp 256 string def

    dup "i386" strstr 0 ne over "x86_64" strstr 0 ne or {
      dup "i386" strstr {
        /32bit_boot_dir exch def
        /64bit_boot_dir 32bit_boot_dir "i386" "x86_64" strreplace
      } {
        /64bit_boot_dir exch def
        /32bit_boot_dir 64bit_boot_dir "x86_64" "i386" strreplace
      } ifelse

      dup "%s/isolinux.cfg" 64bit.tmp sprintf
      64bit.tmp filesize .undef ne { def } { free pop } ifelse

    } {
      /32bit_boot_dir over def
      /64bit_boot_dir exch def
    } ifelse

    64bit.tmp free

    % font.normal setfont
    % 0 400 moveto 32bit_boot_dir print
    % 0 420 moveto 64bit_boot_dir print dtrace

  } {
    pop
  } ifelse
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% input event handling
%
% ( key ) ==> ( input_buffer menu_entry action )
%
% key
%  bit 0-7	ascii
%  bit 8-15	scan code
%  bit 16-32	status bits (ctrl, shift...)
%
% action
%    0:		ok, stay in input loop
%    1:		switch to text mode
%  >=2:		start linux
%
/KeyEvent {
  % timeout

  dup 0 eq { boot.buf buildcmdline 2 return } if

  debug 4 ge {
    % print keycode somewhere
    -1 settransparentcolor
    white setcolor
    500 0 moveto dup print "  " print
  } if

  dup 0xff00 and 16 shl over 0xff and dup 0xe0 eq { pop 0 } if add /key exch def
  16 shr 0xffff and /keystat exch def

  key keyTab eq keyStatus statusShift and 0 ne and {
    /key keyShiftTab def
  } if

  key

  config.keymap .km.map get { mapkey } if

  dup 0xffffff and dup { exch } if pop

  debug 4 ge {
    % print mapped key somewhere
    -1 settransparentcolor
    white setcolor
    500 20 moveto dup print "  " print
  } if

  % some special keys
  debug.input

  % put key through normal input queue
  window.input

  pop

  window.action actExit eq {
    /window.action actNothing def
    "" -1 1 return
  } if

  window.action actCloseInfo eq {
    /window.action actNothing def
    "" -1 3 return
  } if

  window.action actPassword eq {
    /window.action actNothing def
    password.dialog {
      password.dialog .ed.buffer.list get 0 get
    } { "" } ifelse
    -1 3 return
  } if

  window.action actStart eq {
    /window.action actNothing def
    boot.buf buildcmdline 2 return
  } if

  window.action actRedraw eq {
    /window.action actNothing def
    main.redraw
  } if

  window.action actRedrawPanel eq {
    /window.action actNothing def
    panel.show
  } if

  boot.buf menu.indices menu.entry get 0
} def


/bc.cmd cmdlinelength string def

% ( option_string ) ==> ( cmdline menu_entry )
%
% grub:
%   just return
%
% syslinux & lilo:
%   check if the commmand line starts with the current kernel name or
%   "linux"; if not, put the kernel name in front of the command line. (This
%   is to keep compatibility with the old scheme requiring the user to write
%   the kernel name explicitly.)
%
/buildcmdline {
  menu.entry 0 lt { -1 return } if
  menu.entry menu.texts length ge { -1 return } if

  /bc.opts exch def
  /bc.kernel menu.texts menu.entry get def

  grub {

    /bc.addkernel false def

  } {

    /bc.addkernel true def

    [ bc.kernel "linux" ] 0 over length 1 sub 1 exch {
      over exch get

      bc.opts over eq {
        /bc.addkernel false def
      } {
        bc.opts over strstr 1 eq {
           bc.opts over length get ' ' eq {
             /bc.addkernel false def
           } if
        } if
      } ifelse

      pop

      bc.addkernel not { exit } if

    } for

    pop

    % special case: option is identical to label
    bc.kernel "apic" eq { /bc.addkernel true def } if

  } ifelse

  syslinux {
    /add_suffix .undef def

    % set new working directory
    xmenu.bits {
      xmenu.bits .xm_current get 1 eq { 64bit_boot_dir } { 32bit_boot_dir } ifelse
    } { .undef } ifelse
    dup .undef ne {
      dup getcwd ne { chdir } { pop } ifelse
    } {
      pop
    } ifelse
  } if

  % getcwd 0 300 moveto show trace

  bc.addkernel {
    bc.kernel
    dup "linux" eq add_suffix .undef ne and {
      add_suffix exch "%s%s "
    } {
      "%s "
    } ifelse
    bc.cmd sprintf
  } {
    bc.cmd 0 0 put
  } ifelse

  cmdline.hidden "" ne {
    /add_suffix 32 string def

    video.modes.list .undef ne {
      video.modes.list xmenu.video .xm_current get get
      dup .vm_mode get 0 ge {
	dup .vm_height get exch .vm_width get
	",%04d%04d.spl" add_suffix sprintf
      } { pop } ifelse
    } if

    add_suffix 1 add filesize .undef eq {
      add_suffix 0 0 put	% empty string
    } if

    add_suffix {
      /bc.tmp cmdline.hidden length add_suffix length add string def
      cmdline.hidden "initrd" bootopt.find dup .undef ne {
        skipnonspaces
        dup
        dup 0 get over 0 0 put
        add_suffix cmdline.hidden "%s%s" bc.tmp sprintf
        0 exch put
        "%s" bc.tmp dup length add sprintf
      } {
        pop
        bc.tmp cmdline.hidden strcpy pop
      } ifelse

      bc.tmp "%s " bc.cmd dup length add sprintf

      bc.tmp free /bc.tmp .undef def
    } {
      cmdline.hidden "%s " bc.cmd dup length add sprintf
    } ifelse
  } if

  bc.opts "%s " bc.cmd dup length add sprintf

  xmenu.fulloptions {
    fulloptions.option .undef ne {
      fulloptions.option "%s " bc.cmd dup length add sprintf
    } if
  } if

  syslinux {
    video.modes.list .undef ne {
      video.modes.list xmenu.video .xm_current get get .vm_mode get

      dup 0 ge {
	0x200 add "vga=0x%x " bc.cmd dup length add sprintf
      } if
    } if

    config.lang {
      config.lang "en" ne {
        config.lang
        config.lang "_" strstr 0 eq {
          "debian-installer/language=%s "
        } {
          "locale=%s "
        } ifelse
        bc.cmd dup length add sprintf
      } if
    } if

    config.lang "en" ne config.keymap .km.name get "us" ne or {
      /keymap.op keymap.explicit { "=" } { "?=" } ifelse def
      config.keymap .km.name get
      dup "_" strstr dup 0 eq {
        pop
        keymap.op "keyboard-configuration/layoutcode%s%s " bc.cmd dup length add sprintf
      } {
        over over 1 sub 0 put
        over keymap.op "keyboard-configuration/layoutcode%s%s " bc.cmd dup length add sprintf
        add keymap.op "keyboard-configuration/variantcode%s%s " bc.cmd dup length add sprintf
      } ifelse
      config.keymap .km.model get dup {
        keymap.op "keyboard-configuration/modelcode%s%s " bc.cmd dup length add sprintf
      } {
        pop
      } ifelse
    } if

    modes.selected.append .undef ne {
      modes.selected.append "%s " bc.cmd dup length add sprintf
    } if

    modes.selected.replace .undef ne {
      modes.selected.replace "=" strstr dup 0 ne {
	modes.selected.replace exch strndup /bc.tmp exch def  % e.g. "file="
	bc.cmd {
	  dup 0 get 0 eq { exit } if
	  dup getword
	  dup "=" strstr dup 0 ne { over exch 0 put } { pop } ifelse
	  dup bc.tmp eq exch free {
	    bc.tmp free
	    dup strdup /bc.tmp exch def
	    bc.tmp modes.selected.replace "%s%s" 4 index sprintf
	    bc.tmp free /bc.tmp .undef def
	    exch pop exit
	  } if
	  exch pop
	  skipspace
	} loop
	pop
	bc.tmp free /bc.tmp .undef def
      } {
	pop
      } ifelse
    } if
  } if

  xmenu.access {
    access.option .undef ne {
      access.option "%s " bc.cmd dup length add sprintf
    } if
  } if

  xmenu.profile {
    profile.options xmenu.profile .xm_current get get dup "" ne {
      "%s " bc.cmd dup length add sprintf
    } { pop } ifelse
  } if

  timeout.hidden {
    "maybe-ubiquity " bc.cmd dup length add sprintf
  } if

  bc.cmd dropspaces

  debug 3 ge {
    0 0 moveto white setcolor
    bc.cmd print "<<  (press ESC) " print trace
  } if

  bc.cmd menu.indices menu.entry get
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% ( menu_entries_array cmdline_args_array defaultentry ) == > ( )
/MenuInit {
  colorbits 8 le {
    0 setcolor 0 0 moveto screen.size fillrect loadpalette
  } if

  init

  /menu.entry -1 def

  /menu.dentry exch def

  menuconfig.init

  "foreground" getgfxconfig dup .undef ne {
    strtol
    /menu.text.select over def
    /boot.text.options over def
    /boot.text.normal over def
    /panel.high exch def
  } { pop } ifelse
  "background" getgfxconfig dup .undef ne {
    strtol
    /menu.text.normal over def
    /panel.normal over def
    /title.bg exch def
  } { pop } ifelse
  "screen-colour" getgfxconfig dup .undef ne {
    strtol
    /menu.bar.color over def
    /xmenu.dark exch def
  } { pop } ifelse
  "access-options" getgfxconfig dup .undef ne {
    /access.optionnames [
      .undef
      3 index
      {
	skipspace dup 0 get 0 eq { exit } if
	getword exch
      } loop
      pop
    ] def
    pop
  } { pop } ifelse
  "access-options-v1" getgfxconfig dup .undef ne {
    /access.options.v1 exch def
  } { pop } ifelse
  "hidden-timeout" getgfxconfig dup .undef ne {
    % If >= 1, display just the main and access logos and wait for a
    % keypress; if timeout expires, boot without showing the menu.
    % If >= 2, as above, but don't display the splash image (hack for
    % Ubuntu 10.04 LTS, so please don't rely on it permanently).
    strtol
    dup 1 ge { /timeout.hidden true def } if
    2 ge { /timeout.hidden.showsplash false def } if
  } { pop } ifelse

  menu.bar.color setcolor 0 0 moveto screen.size fillrect

  % Hack for Ubuntu 10.04 LTS.  Should be replaced by better logo handling
  % in general.
  timeout.hidden.showsplash not {
    /background.file.orig background.file def
    "blank.pcx" findfile
    /background.file over def setimage
  } if

  % Move the logo down 84 pixels to centre it.  This is obviously a gross
  % hack.  To fix this, we need to move to smaller logo images which are
  % positioned on the screen in code, rather than doing the positioning in
  % the image files.
  timeout.hidden {
    0 0 moveto 0 0 0 84 image
    0 84
  } { 0 0 } ifelse
  moveto 0 0 clip.size image

  /menu.args exch def
  /menu.texts exch def
  /menu.humans exch def
  /menu.indices exch def

  timeout.hidden {
    window.splash
    dup window.init
        window.show
  } {
    window.main
    dup window.init
        window.show

    32+64bit_source not {

      64bit {
        64bit_source not { notimeout /timeout.hidden false def 32bit_popup } if
      } {
        64bit_source { notimeout /timeout.hidden false def 64bit_popup } if
      } ifelse

    } if

    lang.displayed not { panel.lang } if
  } ifelse

  50 sound.setvolume
  "startup.mod" findfile dup .undef ne {
    /player.startup 0 def
    player.startup over mod.load
    player.startup 0 mod.play
    free
  } { pop } ifelse
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Setup boot option input field.
%
% ( ) == > ( )
%
/bootoptions.init {
  window.current .color.fg get setcolor
  window.current .ed.font get setfont

  boot.show { boot.ed edit.hidecursor } if
  menu.args menu.entry get
  bootpromptmap

  boot.show {
    boot.ed over edit.init
    "" ne { boot.ed ' ' edit.input } if
  } {
    "%s" boot.ed 3 get sprintf
  } ifelse

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Split command line into hidden and normal part.
%
% ( cmdline_args ) == > ( new_cmdline_args )
%
/bootpromptmap {
  /cmdline exch def

  cmdline "showopts" getoption
  dup "" eq {
    cmdline.shown 0 0 put
    pop cmdline "%s" cmdline.shown sprintf
    cmdline.hidden 0 0 put
  } {
    "showopts" length add skipspaces
    "%s" cmdline.shown sprintf
    cmdline "%s" cmdline.hidden sprintf
    cmdline.hidden "showopts" getoption 0 0 put
  } ifelse

  cmdline.shown dropspaces
  cmdline.hidden dropspaces

  cmdline.shown
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Get boot option.
%
% ( cmdline option_name ) ==> ( option_start )
%
/getoption {
  /go.name exch def
  /go.cmdline exch def
  /go.pos 0 def
  {
    go.cmdline go.pos add go.name strstr dup {
       1 sub /go.pos exch def

       go.cmdline go.pos iswordboundary
       go.cmdline go.pos go.name length add iswordboundary
       and {
         go.cmdline go.pos add exit
       } {
         /go.pos go.pos 1 add def
       } ifelse
    } {
      pop "" exit
    } ifelse
  } loop
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/redrawmenu {
  menu.visible.entries menu.texts length lt menu.scrollbar and {
    /menu.vsb.height1
      menu.shift
      menu.sb.height mul menu.texts length div
    def

    /menu.vsb.height3
      menu.texts length menu.visible.entries menu.shift add sub
      menu.sb.height mul menu.texts length div
    def

    lightgray setcolor
    menu.sb.x menu.sb.y menu.vsb.height1 add moveto
    menu.sb.width menu.sb.height menu.vsb.height1 menu.vsb.height3 add sub
    fillrect

    menu.vsb.height1 0 ne {
      menu.sb.x menu.sb.y moveto
      currentpoint menu.sb.width menu.vsb.height1 image
    } if

    menu.vsb.height3 0 ne {
      menu.sb.x menu.sb.y menu.sb.height menu.vsb.height3 sub add moveto
      currentpoint menu.sb.width menu.vsb.height3 image
    } if

  } if

  menu.text.normal setcolor

  /x menu.start.x def
  /y menu.start.y def

  0 1 menu.visible.entries 1 sub {
   x y moveto currentpoint menu.bar.width menu.bar.height image
   menu.texts exch menu.shift add get menuitemmap
   x menu.bar.width 2 index strsize pop sub 2 div add
   y menu.text.yofs add
   moveto
   currentfont exch font.large setfont show setfont
   /y y menu.item.height add def
  } for

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% ( entry status ) ==> ( )
%
% status:
%  false	not selected
%  true		selected
%
/MenuSelect {
  /menu.status exch def

  /menu.idx over def

  menu.shift sub
  menu.item.height mul menu.start.y add
  menu.start.x exch
  moveto

  currentpoint
  menu.status {
    menu.bar.color setcolor
    currenttransparency
    70 settransparency
    menu.bar.width menu.bar.height
    fillrect
    settransparency
  } {
    currentpoint menu.bar.width menu.bar.height image
  } ifelse
  moveto

  menu.status {
    menu.text.select
  } {
    menu.text.normal
  } ifelse
  setcolor
  menu.texts menu.idx get menuitemmap
  menu.bar.width over strsize pop sub 2 div menu.text.yofs rmoveto
  currentfont exch font.large setfont show setfont

  menu.status {
    % init boot options
    keepbootoptions .undef eq { bootoptions.init } if

    % set help context
    "main" help.setcontext
    menu.texts menu.idx get
    dup help.findpage "" eq {
      pop
    } {
      help.setcontext
    } ifelse

    % update installation modes menu
    menu.idx modes.filter
  } if

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% ( text errorcode ) ==> ( )
% errorcode:
%   0	normal info
%   1	fatal error
%   2	missing kernel
%   3   disk change
%   4   disk change failed
%

/info.tmpmsg 127 string def

/InfoBoxInit {
  /info.type exch def
  /info.msg exch def

  window.dialog

  info.type 1 eq {
    dup .title.bg red put
    dup .title txt_error_title put
    dup .buttons
    [
      button.reboot button.default actCloseInfo button.setaction
    ] put
  } {
    dup .title txt_info_title put
    dup .buttons
    [
      button.ok button.default actCloseInfo button.setaction
%      button.cancel button.notdefault actCloseInfo button.setaction
    ] put
  } ifelse

  syslinux info.type 3 eq and {
    dup .title txt_change_disk_title put
    0 getinfo 1 add txt_insert_disk info.tmpmsg sprintf
    /info.msg info.tmpmsg def
  } if

  syslinux info.type 4 eq and {
    dup .title txt_change_disk_title put
    1 getinfo 15 not and {
      0 getinfo 1 add
      txt_insert_disk3 info.tmpmsg sprintf
    } {
      0 getinfo 1 add 1 getinfo 1 add
      txt_insert_disk2 info.tmpmsg sprintf
    } ifelse
    /info.msg info.tmpmsg def
  } if

  dup .text info.msg put

  dup window.init
      window.show

} def



% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% ( time ) ==> ( )
% /Timer { pop } def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% ( label correct_password ) ==> ( )
%
/PasswordInit {
  /password.key exch def pop

  /dia window.dialog def

  /password.dialog dia def

  dia .title txt_password_title put
  dia .text "" put

  % Must all be of same size!
  dia .ed.list 1 array put
  dia .ed.buffer.list [ 31 string ] put
  dia .ed.text.list [ txt_password ] put

  dia .ed.focus 0 put

  dia .ed.width 200 put

  dia .ed.font font.normal pwmode put

  dia .buttons
    [ button.ok button.default actPassword button.setaction ]
  put

  dia window.init
  dia window.show

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% ( password ) ==> ( error )
%
% error:
%   true   password ok
%   false  wrong password
%
% ****** FIXME: test result seems to be unused
%
/PasswordDone {

  password.key eq
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% ( text ) == > ( new_text )
% If text is in menu.texts (it should be), return the corresponding item
% from menu.humans. Otherwise, return text as-is.
/menuitemmap {
  0 menu.texts {
    2 index eq { menu.humans over get rot pop exch exit } if
    1 add
  } forall pop
  translate

  % Remove ^ from the string; eventually, this should define hotkeys, but it
  % doesn't yet.
  dup length string /menuitemmap.text exch def
  /menuitemmap.idx 0 def
  {
    dup '^' ne {
      menuitemmap.text menuitemmap.idx rot put
      /menuitemmap.idx inc
    } { pop } ifelse
  } forall
  menuitemmap.text menuitemmap.idx 0 put

  menuitemmap.text
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% ( color0 color1 width height ) ==> ( )
/drawborder {
  currentpoint /db.y0 exch def /db.x0 exch def

  /db.y1 exch 1 sub db.y0 add def
  /db.x1 exch 1 sub db.x0 add def
  /db.col1 exch def
  /db.col0 exch def

  db.x0 db.y1 moveto

  db.col0 setcolor
  db.x0 db.y0 lineto db.x1 db.y0 lineto

  db.col1 setcolor
  db.x1 db.y1 lineto db.x0 db.y1 lineto
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% ( color0 color1 color2 width height ) ==> ( )
% draw frame with shadow
% color0: upper left, color1: lower right, color2: shadow
/drawborder3 {
  currentpoint /db.y0 exch def /db.x0 exch def

  /db.y1 exch 1 sub db.y0 add def
  /db.x1 exch 1 sub db.x0 add def
  /db.col2 exch def
  /db.col1 exch def
  /db.col0 exch def

  db.x0 db.y1 moveto

  db.col0 setcolor
  db.x0 db.y0 lineto db.x1 db.y0 lineto

  db.col1 setcolor
  db.x1 db.y1 lineto db.x0 db.y1 lineto

  db.col2 -1 ne {
    db.col2 setcolor
    1 1 rmoveto
    db.x1 1 add db.y1 1 add lineto
    db.x1 1 add db.y0 1 add lineto
  } if
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% ( color0 color1 width height ) ==> ( )
/drawborder4 {
  3 index 4 1 roll rot dup 3 index 3 index
  currentpoint 6 2 roll
  1 1 rmoveto drawborder
  moveto drawborder
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% center text
% ( text width height ) ==> ( )
/centertext {
  3 -1 roll strsize
  4 2 roll
  4 1 roll exch 4 1 roll sub 2 div neg 3 1 roll sub 2 div neg
} def


% Allocate and define a new color.
%
% ( palette ) ==> ( color )
%
/newcolor {
  colorbits 8 le {
    newcolor.count .undef eq { /newcolor.count 0 def } if
    max_image_colors newcolor.count add
    dup rot setpalette
    /newcolor.count newcolor.count 1 add def
  } if
  def
} def


/boot.pos.init {
  /boot.label.len txt_bootoptions strsize pop def
  /boot.pos { boot.label.len 10 add small_layout { 410 } { 510 } ifelse } def
} def

/init {
  small_layout {
    /head.x 70 def
    /head.y 141 def
  } {
    /head.x 234 def
    /head.y 185 def
  } ifelse

  timeout.init

  % set default language
  "lang" findfile dup {
    /tmp over length 1 add 2 max string def
    tmp exch {
      dup ' ' eq over '\n' eq or { pop pop exit } if
      over exch 0 exch put
      1 add
    } forall
    tmp dup setlang pop setkeymap
    /lang.displayed true def
  } {
    pop
    "en" dup setlang pop setkeymap
  } ifelse

  font.large setfont

  /menu.text.xofs      10 def
  /menu.text.yofs       2 def
  /menu.item.height    fontheight dup 3 div add def
  /menu.bar.height     fontheight menu.text.yofs dup add add def

  font.normal setfont

  /menu.text.normal    lightblue         def
  /menu.text.select    white             def
  /boot.text.options   white             def
  /boot.text.normal    white             def
  /infobox.bg          lightgray         def
  /infobox.text.normal black             def

  /menu.bar.color      black       def

  small_layout {
    /frame1.pos  {  80 208 } def
    /frame1.size { 480 160 } def
    /frame4.pos  { 560  62 } def
  } {
    /frame1.pos  { 160 288 } def
    /frame1.size { 480 170 } def
    /frame4.pos  { 640  62 } def
  } ifelse

  /frame4.size {  22 264 } def

  /menu.start.x frame1.pos pop 10 add def
  /menu.start.y frame1.pos exch pop 12 add def

  /menu.bar.width frame1.size pop 20 sub def
  /menu.max.entries 8 def
  /menu.scrollbar false def

  boot.pos.init

  /boot.buf.size cmdlinelength def
  /boot.buf boot.buf.size string def

  % no boot options line initially
  /boot.show false def

  /ms.size     {  22  22 } def
  /ms.up       {   0 480 }  def
  /ms.down     {  23 480 }  def

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Some special debug & test keys.
%
% ( key_in ) ==> ( key_out )
%
/debug.input {
  dup keyShiftF4 eq debug 3 ge and {
    currentcolor white setcolor
    currentpoint 0 0 moveto
    64 string biosmem "mem = %u  " 2 index sprintf dup show free
    moveto setcolor
    pop 0
  } if

  dup keyShiftF3 eq syslinux and debug 3 ge and {
    currentcolor white setcolor
    currentpoint 0 0 moveto
    "eject  " print bootdrive eject print
    moveto setcolor
    pop 0
  } if

  dup keyShiftF5 eq syslinux and debug 3 ge and {
    currentcolor white setcolor
    currentpoint 100 0 moveto
    bootdrive print
    moveto setcolor
    pop 0
  } if

  dup keyShiftF8 eq debug 3 ge and {
    currentcolor white setcolor

    currentpoint 300 0 moveto
    0 memsize print "/" print print "  " print
    moveto

    currentpoint 300 20 moveto
    1 memsize print "/" print print "  " print
    moveto

    currentpoint 300 40 moveto
    2 memsize print "/" print print "  " print
    moveto

    setcolor
    pop 0
  } if

  dup keyShiftF9 eq debug 3 ge and {
    dumpmem
    pop 0
  } if

  dup keyShiftF10 eq {
    /debug debug 1 add def ""
    pop 0
  } if

  dup keyShiftF11 eq {
    /transp transp 0x10 sub 0 max def
    pop 0
  } if

  dup keyF11 eq debug 1 ge and {

    dtrace

    0 1 screen.size exch pop {
      0 1 screen.size pop {
        over moveto
        currentpoint 8 shl screen.size exch pop div 8 shl exch 8 shl screen.size pop div add
        screen.size add currentpoint add sub 8 shl screen.size add div 2 div 16 shl add
        setcolor
        putpixel
      } for
      pop
    } for

    pop 0
  } if

  dup keyF12 eq debug 3 ge and {
    % mouse support testing

    1 keepmode

    /mouse xxx def
    red setcolor
    mouse .undef eq {
      700 580 moveto "no mouse" show
    } {
      {
        mouse getdword 16 shl 16 shr screen.size pop 2 div add
        mouse getdword 16 shr neg screen.size exch pop 2 div add
        moveto 4 4 fillrect

        mouse 4 add getdword 7 and
        dup 1 and { red setcolor } if
        dup 2 and { blue setcolor } if
        dup 4 and { green setcolor } if

        3 and 3 eq { exit } if

      } loop
    } ifelse
    pop 0
  } if

  dup keyShiftF12 eq {
    /transp transp 0x10 add 0x100 min def
    pop 0
  } if

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show exit popup.
%
% ( ) ==> ( )
%
/exit_popup {
  window.dialog

  dup .title txt_exit_title put
  dup .text txt_exit_dialog put
  dup .buttons [
    button.ok button.default actExit button.setaction
    button.cancel button.notdefault actNothing button.setaction
  ] put
  dup window.init
      window.show

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show help window.
%
% ( ) ==> ( )
%
/show_help {
  window.help
  
  dup window.init
  window.show

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show dvd popup.
%
% ( ) ==> ( )
%
/dvd_popup {
  window.dialog

  dup .title txt_dvd_warning_title put
  dup .text txt_dvd_warning2 put
  dup .buttons [
%    button.eject    button.default    actEject actNoClose or button.setaction
    button.continue button.default actNothing button.setaction
  ] put
  dup window.init
      window.show

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Warn that we are about to install 32bit software on a 64bit system.
%
% ( ) ==> ( )
%
/32bit_popup {
  window.dialog

  dup .title "Cool computer, but..." put
  dup .text "You are about to install 32-bit software on a 64-bit computer." put
  dup .buttons [
    button.continue button.default actNothing button.setaction
  ] put
  dup window.init
      window.show

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Warn that we are about to install 64bit software on a 32bit system.
%
% ( ) ==> ( )
%
/64bit_popup {
  window.dialog

  dup .title "Cool software, but..." put
  dup .text "This is a 32-bit computer. You cannot use 64-bit software on it." put
  dup .buttons [
    button.reboot button.default actReboot actNoClose or button.setaction
  ] put
  dup window.init
      window.show

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Warn that we don't have enough memory.
%
% ( ) ==> ( )
%
/nomem_popup {
  window.dialog

  dup .title "Cool computer, but..." put
  dup .text
    "Sorry, but SUSE Linux 10.1 LiveCD needs at least 256MB main memory."
  put
  dup .buttons [
    button.reboot button.default actReboot actNoClose or button.setaction
  ] put
  dup window.init
      window.show

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show "power off" popup.
%
% ( ) ==> ( )
%
/power_off {
  window.dialog

  dup .title txt_power_off_title put
  dup .text txt_power_off put
  dup .buttons [
    button.ok     button.notdefault    actPowerOff actNoClose or button.setaction
    button.cancel button.default       actNothing                button.setaction
  ] put
  dup window.init
      window.show

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Search for option in cmdline.
% Returns .undef if not found.
%
% ( cmdline option_name ) ==> ( option_start )
%
/bootopt.find {
  /bo.opt exch def
  /bo.cmdline exch def

  {
    bo.cmdline bo.opt strstr
    dup {
      dup 1 eq {
        true
      } {
        dup 2 sub bo.cmdline exch get ' ' eq
      } ifelse

      {
        bo.cmdline over bo.opt length add 1 sub get
        dup '=' eq
        over ' ' eq or
        exch 0 eq or
      } {
        false
      } ifelse

      bo.cmdline rot add exch

      {
        1 sub exit
      } {
        /bo.cmdline exch def
      } ifelse
    } {
      pop
      .undef exit
    } ifelse
  } loop

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Global variables.
%

/transp 0 def

640 480 16 findmode setmode not {
  640 480 32 findmode setmode not {
    640 400 4 findmode setmode not {
      false .end
    } if
  } if
} if

% /with_transp vscreen.size exch pop screen.size exch pop div 1 gt colorbits 8 gt and def

/clip.size { screen.size } def

/small_layout screen.size pop 640 eq def

/access.file "access.pcx" findfile def
access.file dup .undef ne {
  setimage image.size /access.height exch def /access.width exch def
} {
  pop /access.height .undef def /access.height .undef def
} ifelse

"splash.pcx" findfile
dup .undef eq { pop "back.jpg" findfile } if
/background.file over def setimage loadpalette
/max_image_colors image.colors def

% 7 dup 0xffffff setpalette setcolor

% color & font definitions must be global

/black               0x000000          newcolor
/white               0xffffff          newcolor
/blue                0x0000a0          newcolor
/red                 0xc00000          newcolor
/green               0x009000          newcolor
/yellow              0xffff20          newcolor
/lightgray           0xd8deed          newcolor
/dotcolor            0xa9bbcd          newcolor
/lightdotcolor       0xeff2f6          newcolor
/somegreen           0x78a424          newcolor

/lightblue 0x7da3d1 def

%% include theme_colors.inc

/font.normal "16x16.fnt" findfile def
/font.large font.normal def

/cmdline.hidden cmdlinelength string def
/cmdline.shown cmdlinelength string def

/debug 0 def

check_arch_boot_dir



