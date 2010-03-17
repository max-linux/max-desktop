% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Panel handling.
%
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Some global vars.
%
/panel.size     { clip.size pop 24 } def
/panel.y        { clip.size exch pop panel.size exch pop sub 10 sub } def
/panel.text.y	{ panel.y 5 add } def
/panel.font	font.normal def
/panel.normal	lightblue def
/panel.high	white def
/panel.bg	lightgray def
/panel.border	lightdotcolor def

/.panel.full    1 def
/.panel.short   2 def
/.panel.both    .panel.full .panel.short or def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Init panel.
%
% ( ) ==> ( )
%
/panel.init {

  /panel.back 0 panel.y moveto panel.size savescreen def

  % define panel layout
  /panel.template [

    % [ key x label show_func width_func update_func init_func ]

    help.getmessages "" ne {
      [ .panel.both [ keyF1 0 "F1" /panel.help /panel.help.width /panel.help.update .undef ] ]
    } if

    syslinux {

      [ .panel.both [ keyF2 0 "F2" /panel.lang /panel.lang.width /panel.lang.update /lang.init ] ]
      [ .panel.both [ keyF3 0 "F3" /panel.keymap /panel.keymap.width /panel.keymap.update /keymap.init ] ]
      [ .panel.both [ keyF4 0 "F4" /panel.modes /panel.modes.width /panel.modes.update /modes.init ] ]
      [ .panel.both [ keyF5 0 "F5" /panel.access /panel.access.width /panel.access.update /access.init ] ]

      [ .panel.short [ keyF6 0 "F6" /panel.options /panel.fulloptions.width /panel.fulloptions.update .undef ] ]

      [ .panel.full [ keyF6 0 "F6" /panel.fulloptions /panel.fulloptions.width /panel.fulloptions.update /fulloptions.init ] ]

      32+64bit_source {
	64bit {
	  32bit_boot_dir .undef ne
	  64bit_boot_dir .undef ne
	  32bit_boot_dir 64bit_boot_dir ne
	  and and {
	    [ .panel.full [ keyF7 0 "F7" /panel.bits /panel.bits.width /panel.bits.update /bits.init ] ]
	  } if
	} if
      } if

      xmenu.bits .undef eq { bits.init } if

    } {

      [ .panel.both [ keyF2 0 "F2" /panel.lang /panel.lang.width /panel.lang.update /lang.init ] ]
      "profiles" findfile dup {
        /profile.data exch def
        [ .panel.both [ keyF3 0 "F3" /panel.profile /panel.profile.width /panel.profile.update /profile.init ] ]
      } { pop } ifelse

    } ifelse

  ] def

  % initialize all
  panel.template { 1 get 6 get dup .undef ne { exec } { pop } ifelse } forall

  /panel.full [
    panel.template {
      dup 0 get .panel.full and 0 ne { 1 get } { pop } ifelse
    } forall
  ] def

  /panel.short [
    panel.template {
      dup 0 get .panel.short and 0 ne { 1 get } { pop } ifelse
    } forall
  ] def

  /panel panel.short def

  panel.show
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show panel.
%
% ( ) ==> ( )
%
/panel.show {
  panel.font setfont

%  panel.bg setcolor
  0 panel.y moveto
%  currentpoint
  panel.back restorescreen
%  currenttransparency
%  50 settransparency
%  panel.size fillrect
%  settransparency
%  moveto

%  panel.border setcolor
%  0 -1 rmoveto
%  0 1 clip.size pop 3 div { pop putpixel 3 0 rmoveto } for

  % don't change xmenu
  /xmenu xmenu

  /panel.x 0 def
  panel { dup {
    dup 1 panel.x put
    dup 4 get exec over 2 get strsize pop add 20 add panel.x add /panel.x exch def
    dup 5 get exec
  } { pop } ifelse } forall

  def

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Handle keyboard input.
%
% ( key_in ) ==> ( key_out )
%
/panel.input {
  panel { dup {
    dup 0 get 2 index eq { 3 get exec pop 0 exit } { pop } ifelse
  } { pop } ifelse } forall
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Draw panel entry label and move to text field.
%
% ( panel ) => ( )
%
/panel.text.moveto {
  /panel.tmp.x over 1 get 10 add def
  /panel.tmp.F exch 2 get def
  
  panel.tmp.x panel.text.y moveto
  panel.high setcolor panel.tmp.F show
  panel.normal setcolor

  6 0 rmoveto
} def


