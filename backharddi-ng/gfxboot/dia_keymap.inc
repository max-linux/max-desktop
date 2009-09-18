% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Keymap selection dialog.
%
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


/keymap.inited false def
/keymap.explicit false def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Build keymap list.
%
% ( ) ==> ( )
%
/keymap.init {
  /xmenu.keymap .xm_size array def
  /xmenu xmenu.keymap def
  /keymap.inited true def

  xmenu .xm_current 0 put
  xmenu .xm_list [ keymaps { .km.display get } forall ] put

  xmenu .xm_title /txt_keymap put

  % setkeymap is called by init before panel.init, so config.keymap must be
  % set by now
  keymap.langchanged

  pmenu.init
} def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Language changed; update keymap.
%
% ( ) ==> ( )
%
/keymap.langchanged {
  keymap.inited {
    /keymap.tmp.cnt 0 def
    keymaps {
      config.keymap eq {
        xmenu.keymap .xm_current keymap.tmp.cnt put exit
      } if
      /keymap.tmp.cnt inc
    } forall
  } if
} def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Update keymap.
%
% ( ) ==> ( )
%
/keymap.update {
  /xmenu xmenu.keymap def

  /config.keymap keymaps xmenu .xm_current get get def
  /keymap.explicit true def

  window.action actRedraw eq { pmenu.update } if
} def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show keymap menu.
%
% ( ) => ( )
%
/panel.keymap {
  "keymap" help.setcontext

  window.xmenu
  dup .xmenu xmenu.keymap put
  dup .xmenu.update /keymap.update put
  dup window.init
      window.show
} def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Return width of panel entry.
%
% ( ) => ( width )
%
/panel.keymap.width {
  /xmenu xmenu.keymap def

  pmenu.width
} def

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Redraw panel entry.
%
% ( panel ) => ( )
%
/panel.keymap.update {
  /xmenu xmenu.keymap def

  pmenu.panel.update
} def
