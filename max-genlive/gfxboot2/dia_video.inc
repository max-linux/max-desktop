% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Video mode selection dialog.
%
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Some global vars.
%
% video mode array fields
/.vm_mode   0 def
/.vm_width  1 def
/.vm_height 2 def
/.vm_depth  3 def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Create sort key for video modes.
% Supports up to 16383x16383x56 in multiples of 8 bits of depth.
%
% ( vm_index )  ==> ( sort_index )
%
/vmsortindex {
  video.modes.list exch get
  dup
  .vm_width get 14 shl
  over .vm_height get add 3 shl
  exch .vm_depth get 3 shr add
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Swap video mode entries.
% (Helper for video mode sorting.)
%
% ( vm_index_1 vm_index_2 )  ==> ( )
%
/vmsortexch {
  over video.modes.list exch get
  over video.modes.list exch get
  video.modes.list
  5 -1 roll rot put
  video.modes.list 3 1 roll put
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Build video mode list.
%
% ( ) ==> ( )
%
/video.init {
  /xmenu.video .xm_size array def

  /xmenu xmenu.video def

  % build list of video modes
  /video.modes.list [
    [ -1 0 0 0 ]	% special: default VGA mode

    0 1 videomodes {
      videomodeinfo dup .undef eq {
        pop pop pop pop
      } {
        [
          over 0xbfff and 6 2 roll
          0x4000 and		% fb support
          over 0x10 ge and	% >= 16 color bits
          2 index 400 ge and	% height >= 400
          3 index 600 ge and	% width >= 600
        { ] } { pop pop pop pop pop } ifelse
      } ifelse
    } for

  ] def

  % sort video.modes.list

  video.modes.list length 3 gt {
    0 1 video.modes.list length 2 sub {
      dup 1 add 1 video.modes.list length 1 sub {
        over vmsortindex over vmsortindex gt {
          over over vmsortexch
        } if
        pop
      } for
      pop
    } for
  } if

  % uniquify video.modes.list

  video.modes.list length 1 gt {
    /video.modes.list [
      video.modes.list 0 get
      1 1 video.modes.list length 1 sub {
        video.modes.list over 1 sub get
        video.modes.list 2 index get

        over .vm_width get over .vm_width get ne
        2 index .vm_height get 2 index .vm_height get ne or
        rot .vm_depth get rot .vm_depth get ne or {
          video.modes.list exch get
        } {
          pop
        } ifelse
      } for
    ] def
  } if

  % create mode strings

  /video.modes.text [
    video.modes.list {
      dup .vm_width get
      dup 0 eq {
        pop pop "VGA"
      } {
        over .vm_depth get rot .vm_height get rot
        "%d x %d x %d" 32 string dup 6 1 roll sprintf
      } ifelse
    } forall
  ] def

  % add to menu

  xmenu .xm_list video.modes.text put

  % select largest mode the monitor supports

  monitorsize exch 800 max exch 600 max		% at least 800x600

  xmenu .xm_current 0 put
  % video.modes.list {
  %   dup .vm_width get 3 index le
  %   exch .vm_height get 3 index le and {
  %     xmenu .xm_current over over get 1 add put
  %   } {
  %     exit
  %   } ifelse
  % } forall

  pop pop

  pmenu.init
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Update video mode.
%
% ( ) ==> ( )
%
/video.update {
  /xmenu xmenu.video def

  /window.action actRedrawPanel def

  pmenu.update
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show video menu.
%
% ( ) => ( )
%
/panel.video {
  "videomode" help.setcontext

  window.xmenu
  dup .xmenu xmenu.video put
  dup .xmenu.update /video.update put
  dup window.init
      window.show
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Return width of video entry.
%
% ( ) => ( width )
%
/panel.video.width {
  /xmenu xmenu.video def

  pmenu.width
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Redraw panel entry.
%
% ( panel ) => ( )
%
/panel.video.update {
  /xmenu xmenu.video def

  pmenu.panel.update
} def


