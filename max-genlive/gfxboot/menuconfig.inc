% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Read syslinux configuration to get human-readable names for menu items, and
% to decide whether to hide menu items. The configuration used is the same as
% that used by the syslinux simple menu system, so you can use the same
% configuration file for both.
%
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

% ( str2 str1 -- str )
/menuconfig.append {
  over length over length add string /menuconfig.append.tmp exch def
  "%s%s" menuconfig.append.tmp sprintf
  menuconfig.append.tmp
} def

% Read a configuration file. If file_name is .undef, look in some standard
% places.
% ( file_name -- file_contents )
/menuconfig.find {
  /menuconfig.find.tmp exch def
  menuconfig.find.tmp {
    [
      menuconfig.find.tmp "/" menuconfig.append
      menuconfig.find.tmp "/syslinux/" menuconfig.append
      menuconfig.find.tmp "/isolinux/" menuconfig.append
      menuconfig.find.tmp "/boot/isolinux/" menuconfig.append
    ]
  } {
    [
      "/syslinux.cfg"
      "/syslinux/syslinux.cfg"
      "/isolinux.cfg"
      "/isolinux/isolinux.cfg"
      "/boot/isolinux/isolinux.cfg"
    ]
  } ifelse
  {
    dup filesize .undef ne { findfile cvs return } if
    pop
  } forall
  .undef
} def

% Set the human-readable text for the current menu item.
% ( string -- )
/menuconfig.sethuman {
  menuconfig.curlabel .undef eq { pop return } if

  /menuconfig.human exch def

  /menuconfig.idx 0 def
  menuconfig.entries {
    menuconfig.curlabel eq {
      menuconfig.humans menuconfig.idx menuconfig.human put
      return
    } if
    /menuconfig.idx inc
  } forall
} def

% Remove the current menu item from the menu.
% ( -- )
/menuconfig.hidelabel {
  menuconfig.curlabel .undef eq { return } if

  /menuconfig.idx 0 def
  menuconfig.entries {
    menuconfig.curlabel eq {
      % Copy everything up to here into new arrays.
      /menuconfig.newargs menuconfig.args length 1 sub array def
      /menuconfig.newentries menuconfig.entries length 1 sub array def
      /menuconfig.newhumans menuconfig.humans length 1 sub array def
      menuconfig.idx 0 ne {
        0 1 menuconfig.idx 1 sub {
          menuconfig.args over get
          menuconfig.newargs 2 index rot put
          menuconfig.entries over get
          menuconfig.newentries 2 index rot put
          menuconfig.humans over get
          menuconfig.newhumans 3 1 roll put
        } for
      } if

      menuconfig.idx menuconfig.entries length 1 sub lt {
        % Slide everything else down one place.
        menuconfig.idx 1 menuconfig.entries length 2 sub {
          menuconfig.args over 1 add get
          menuconfig.newargs 2 index rot put
          menuconfig.entries over 1 add get
          menuconfig.newentries 2 index rot put
          menuconfig.humans over 1 add get
          menuconfig.newhumans 3 1 roll put
        } for
      } if

      /menuconfig.args menuconfig.newargs def
      /menuconfig.entries menuconfig.newentries def
      /menuconfig.humans menuconfig.newhumans def

      return
    } if
    /menuconfig.idx inc
  } forall
} def

% ( file_name -- )
/menuconfig.parse {
  % load configuration file; return if not found
  menuconfig.find dup .undef eq { pop return } if

  dup {
    % skip over any NULs we've left behind; exit if there's really no more
    % memory left to go
    {
      dup cvp length 0 eq { exit } if
      dup 0 get 0 ne { exit } if
      1 add
    } loop
    dup cvp length 0 eq { exit } if

    % break lines at \r or \n
    0 1 2 index length 1 sub {
      over over get
      dup '\x0d' eq exch '\n' eq or { over exch 0 put exit } { pop } ifelse
    } for

    skipspace getword tolower
    dup "include" eq {
      over skipspace getword dup menuconfig.parse
    } if
    dup "label" eq {
      over skipspace strdup /menuconfig.curlabel exch def
    } if
    dup "menu" eq {
      over skipspace getword tolower
      dup "label" eq { over skipspace strdup menuconfig.sethuman } if
      dup "hide" eq { menuconfig.hidelabel } if
      % items that switch between syslinux menus are unlikely to be useful
      % in gfxboot
      dup "goto" eq { menuconfig.hidelabel } if
      dup "exit" eq { menuconfig.hidelabel } if
      free rot pop exch
    } if
    dup "config" eq {
      % used for help menu item, not useful in gfxboot
      menuconfig.hidelabel
    } if
    free

    % skip to end of line
    dup length add
  } loop pop
  free
} def

% ( menu_entries_array menu_args_array -- menu_humans_array menu_entries_array menu_args_array )
% The returned menu_args_array (kernel parameters), menu_entries_array
% (identifiers), and menu_humans_array (human-readable names) will have any
% hidden entries removed.
/menuconfig.init {
  dup length array /menuconfig.args exch def
  over length array /menuconfig.entries exch def
  over length array /menuconfig.humans exch def

  % copy input arrays; initialise menuconfig.humans
  /menuconfig.idx 0 def
  {
    menuconfig.args menuconfig.idx rot put
    /menuconfig.idx inc
  } forall
  /menuconfig.idx 0 def
  {
    dup menuconfig.entries menuconfig.idx rot put
    menuconfig.humans menuconfig.idx rot put
    /menuconfig.idx inc
  } forall

  % we only understand syslinux configuration
  syslinux not {
    menuconfig.humans menuconfig.entries menuconfig.args return
  } if

  /menuconfig.curlabel .undef def

  .undef menuconfig.parse

  menuconfig.humans menuconfig.entries menuconfig.args
} def
