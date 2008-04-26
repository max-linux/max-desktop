% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Panel installation modes selector.
%
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


/modes.selected.append .undef def
/modes.selected.replace .undef def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Initialise installation modes menu.
%
% ( ) ==> ( )
%
/modes.init {
  /xmenu.modes .xm_size array def
  /xmenu xmenu.modes def

  xmenu .xm_current 0 put

  /modes.gfxconfig listgfxconfig def

  /modes.rawlabels [
    modes.gfxconfig {
      dup 0 get 6 strndup dup "label " eq exch free { 1 get } { pop } ifelse
    } forall
  ] def
  /modes.append [
    modes.gfxconfig {
      0 get /modes.tmp.key exch def
      modes.tmp.key 6 strndup dup "label " eq exch free {
	modes.tmp.key 6 add dup length 8 add string /modes.tmp.str exch def
	"append %s" modes.tmp.str sprintf
	modes.tmp.str getgfxconfig
	modes.tmp.str free
      } if
    } forall
  ] def
  /modes.replace [
    modes.gfxconfig {
      0 get /modes.tmp.key exch def
      modes.tmp.key 6 strndup dup "label " eq exch free {
	modes.tmp.key 6 add dup length 9 add string /modes.tmp.str exch def
	"replace %s" modes.tmp.str sprintf
	modes.tmp.str getgfxconfig
	modes.tmp.str free
      } if
    } forall
  ] def
  /modes.applies [
    modes.gfxconfig {
      0 get /modes.tmp.key exch def
      modes.tmp.key 6 strndup dup "label " eq exch free {
	modes.tmp.key 6 add dup length 9 add string /modes.tmp.str exch def
	"applies %s" modes.tmp.str sprintf
	modes.tmp.str getgfxconfig
	modes.tmp.str free
      } if
    } forall
  ] def
  modes.rawlabels length 0 eq {
    /modes.rawlabels [ txt_mode_normal ] def
    /modes.append [ .undef ] def
    /modes.replace [ .undef ] def
    /modes.applies [ .undef ] def
  } if
  /modes.map .undef def

  menu.entry modes.filter

  xmenu .xm_title /txt_modes put
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Update installation modes status.
%
% ( ) ==> ( )
%
/modes.update {
  /xmenu xmenu.modes def

  modes.map xmenu .xm_current get get
  modes.append over get /modes.selected.append exch def
  modes.replace over get /modes.selected.replace exch def

  /window.action actRedrawPanel def

  pmenu.update
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Filter displayed modes when a menu item changes.
%
% ( index ) ==> ( )
%
/modes.filter {
  modes.rawlabels .undef eq { pop return } if

  menu.texts exch get
  dup length 3 add string /modes.tmp.label exch def
  " %s " modes.tmp.label sprintf

  xmenu % save
  /xmenu xmenu.modes def

  modes.map {
    /modes.tmp.oldidx modes.map xmenu .xm_current get get def
  } {
    /modes.tmp.oldidx .undef def
  } ifelse

  modes.map free
  % mapping of current menu index to underlying option index
  /modes.map [
    0 1 modes.rawlabels length 1 sub {
      modes.applies over get dup .undef ne {
	dup length 3 add string /modes.tmp.applies exch def
	" %s " modes.tmp.applies sprintf
	modes.tmp.applies modes.tmp.label strstr 0 eq { pop } if
	modes.tmp.applies free
      } {
	pop
      } ifelse
    } for
  ] def

  modes.tmp.label free

  xmenu .xm_list get free
  xmenu .xm_list [
    modes.map { modes.rawlabels exch get translate } forall
  ] put

  xmenu .xm_current 0 put
  0 1 modes.map length 1 sub {
    modes.map over get modes.tmp.oldidx eq {
      xmenu .xm_current rot put exit
    } {
      pop
    } ifelse
  } for

  pmenu.init

  /xmenu exch def % restore
} def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show window.
%
% ( ) ==> ( )
%
/panel.modes {
  "modes" help.setcontext

  window.xmenu
  dup .xmenu xmenu.modes put
  dup .xmenu.update /modes.update put
  dup window.init
      window.show
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Return width of panel entry.
%
% ( ) => ( width )
%
/panel.modes.width {
  /xmenu xmenu.modes def

  pmenu.width
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Redraw panel entry.
%
% ( panel ) => ( )
%
/panel.modes.update {
  /xmenu xmenu.modes def

  pmenu.panel.update
} def
