% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Language selection dialog.
%
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Some global vars.
%

% fallback if there is no "langlist"
/lang.items [ "en" ] def
/lang.names [ "English" ] def
/lang.displayed false def

/.la.language 0 def
/.la.locale   1 def
/.la.name     2 def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Get language name.
%
% Returns locale if not found.
%
% ( locale ) ==> ( name )
%
/lang.getdefname {
  lang.defaultnames {
    dup .la.language get 2 index eq { .la.name get exch pop exit } { pop } ifelse
  } forall
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Get locale code (ll_CC).
%
% Returns locale if not found.
%
% ( locale ) ==> ( ll_CC )
%
/lang.getlocale {
  lang.defaultnames {
    dup .la.language get 2 index eq { .la.locale get exch pop exit } { pop } ifelse
  } forall
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Parse "langlist" file.
%
% ( ) ==> ( )
%
/lang.parsedata {
  /lang.default 0 def

  "langlist" findfile dup { /lang.data exch def } { pop return } ifelse

  /la.tmp.datalen lang.data length def
  /la.tmp.str lang.data cvs def
 
  la.tmp.datalen 0 eq { return } if
  la.tmp.str la.tmp.datalen 1 sub get '\n' ne { return } if

  '\n' seteotchar

  /lang.items [

    /la.tmp.len 0 def
    /la.tmp.cnt 0 def
    {
      la.tmp.str la.tmp.len add strdup
      dup dup length 0 put
      /la.tmp.len over length 1 add la.tmp.len add def

      dup 0 get '*' eq { 1 add /lang.default la.tmp.cnt def } if

      la.tmp.len la.tmp.datalen ge { exit } if

      /la.tmp.cnt inc
    } loop

  ] def

  ' ' seteotchar

  /lang.names [

    lang.items {

      dup dup length add

      dup 0 get {
        dup 0 0 put 1 add
        exch pop
      } {
        pop lang.getdefname
      } ifelse

    } forall

  ] def

  0 seteotchar

} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Build language list.
%
% ( ) ==> ( )
%
/lang.init {
  /xmenu.lang .xm_size array def
  /xmenu xmenu.lang def

  lang.parsedata

  xmenu .xm_current lang.default put
  xmenu .xm_list lang.names put

  xmenu .xm_title /txt_language put

  % make menu smaller if there are more than 19 language entries
  %lang.items length 19 gt {
  %  xmenu .xm_vspace 2 put
  %} if

  % start with current lang

  /la.tmp.found false def

  /la.tmp.cnt 0 def
  lang.items {
    config.lang eq {
      xmenu .xm_current la.tmp.cnt put /la.tmp.found true def exit
    } if
    /la.tmp.cnt inc
  } forall

  la.tmp.found not {
    % Try without the country code.
    config.lang "_" strstr dup 0 ne {
      1 sub
      config.lang strdup dup rot 0 put
      /la.tmp.cnt 0 def
      lang.items {
        over eq {
          xmenu .xm_current la.tmp.cnt put exit
        } if
        /la.tmp.cnt inc
      } forall
      free
    } {
      pop
    } ifelse
  } if

  pmenu.init
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Update language.
%
% ( ) ==> ( )
%
/lang.update {
  /xmenu xmenu.lang def

  lang.items xmenu .xm_current get get
  dup
  setlang { /window.action actRedraw def } if
  setkeymap
  keymap.langchanged

  window.action actRedraw eq { pmenu.update } if
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Clean up after language menu has been undrawn.
%
% ( ) => ( )
%
/lang.cleanup {
  /lang.displayed true def
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show language menu.
%
% ( ) => ( )
%
/panel.lang {
  "keytable" help.setcontext

  window.xmenu
  dup .xmenu xmenu.lang put
  dup .xmenu.update /lang.update put
  dup .xmenu.cleanup /lang.cleanup put
  dup window.init
      window.show
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Return width of panel entry.
%
% ( ) => ( width )
%
/panel.lang.width {
  /xmenu xmenu.lang def

  pmenu.width
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Redraw panel entry.
%
% ( panel ) => ( )
%
/panel.lang.update {
  /xmenu xmenu.lang def

  pmenu.panel.update
} def


