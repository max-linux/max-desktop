% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Accessibility mode selector.
%
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


/access.option .undef def

/.access.none 0 def
/.access.v1 1 def
/.access.v2 2 def
/.access.v3 3 def
/.access.brltty 4 def
/.access.m1 5 def
/.access.m2 6 def

/access.alloptionnames [
  .undef
  "v1" "v2" "v3" "brltty"
  "m1" "m2"
] def

/access.optionnames [ access.alloptionnames { } forall ] def

% Custom kernel options.  Feel free to extend as needed.  Keep this in sync
% with MenuInit.
/access.options.v1 "access=v1" def

/access.alltexts [
  /txt_access_none
  /txt_access_v1 /txt_access_v2 /txt_access_v3 /txt_access_brltty
  /txt_access_m1 /txt_access_m2
] def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Initialise accessibility menu.
%
% ( ) ==> ( )
%
/access.init {
  /xmenu.access .xm_size array def
  /xmenu xmenu.access def

  xmenu .xm_current .access.none put

  xmenu .xm_list [
    access.optionnames {
      0 1 access.alloptionnames length 1 sub {
        over access.alloptionnames 2 index get eq {
          access.alltexts exch get exch pop exit
        } {
          pop
        } ifelse
      } for
    } forall
  ] put

  /access.optionmap [
    0 1 access.optionnames length 1 sub {
      0 1 access.alloptionnames length 1 sub {
        access.optionnames 2 index get access.alloptionnames 2 index get eq {
          [ 2 index rot ] exch exit
          % leaves [ optionindex alloptionindex ] optionindex
        } {
          pop
        } ifelse
      } for
      pop
    } for
  ] def

  xmenu .xm_title /txt_access put

  /access.soundfiles [
    access.optionnames {
      dup .undef ne {
        /access.tmpfilename 32 string def
        "access%s.mod" access.tmpfilename sprintf
        access.tmpfilename findfile
      } if
    } forall
  ] def

  pmenu.init
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Update accessibility status.
%
% ( ) ==> ( )
%
/access.update {
  /xmenu xmenu.access def

  access.optionnames xmenu .xm_current get get dup .undef eq {
    pop
    /access.option .undef def
  } {
    dup "brltty" eq {
      pop /access.option "braille=ask" def
    } {
      dup "v1" eq {
        pop /access.option access.options.v1 def
      } {
        dup length 7 add string /access.option exch def
        "access=%s" access.option sprintf
      } ifelse
    } ifelse
  } ifelse

  /window.action actRedrawPanel def

  pmenu.update
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Select an accessibility menu item.
%
% ( ) ==> ( )
%
/access.select {
  /xmenu xmenu.access def

  access.soundfiles xmenu .xm_current get get dup .undef ne {
    /player.access 1 def
    player.access exch mod.load
    player.access 0 mod.play
  } { pop } ifelse
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Handle keyboard input.
%
% ( key_in ) ==> ( key_out )
%
/access.input {
  dup 0 eq { return } if

  dup 0xff and '0' sub
  dup 0 lt { return } if

  dup access.optionmap {
    dup 1 get 2 index eq {
      0 get dup xmenu .xm_list get length lt {
        xmenu.select
        pop
        0
      } {
        pop
      } ifelse
    } {
      pop
    } ifelse
  } forall
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show window.
%
% ( ) => ( )
%
/panel.access {
  "access" help.setcontext

  window.xmenu
  dup .xmenu xmenu.access put
  dup .xmenu.update /access.update put
  dup .xmenu.select /access.select put
  dup .xmenu.input /access.input put
  dup window.init
      window.show
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Return width of panel entry.
%
% ( ) => ( width )
%
/panel.access.width {
  /xmenu xmenu.access def

  pmenu.width
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Redraw panel entry.
%
% ( panel ) => ( )
%
/panel.access.update {
  /xmenu xmenu.access def

  pmenu.panel.update
} def
