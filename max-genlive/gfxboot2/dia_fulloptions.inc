% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Panel boot options selector.
%
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


/fulloptions.option .undef def

/.fulloptions.expert 0 def
/.fulloptions.acpi_off 1 def
/.fulloptions.noapic 2 def
/.fulloptions.nolapic 3 def
/.fulloptions.edd_on 4 def
/.fulloptions.nodmraid 5 def
/.fulloptions.nomodeset 6 def
/.fulloptions.free 7 def

% These strings are intentionally not translatable.
/txt_acpi_off "acpi=off" def
/txt_noapic "noapic" def
/txt_nolapic "nolapic" def
/txt_edd_on "edd=on" def
/txt_nodmraid "nodmraid" def
/txt_nomodeset "nomodeset" def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Initialise boot options menu.
%
% ( ) ==> ( )
%
/fulloptions.init {
  /xmenu.fulloptions .xm_size array def
  /xmenu xmenu.fulloptions def

  is_live {
    /.fulloptions.acpi_off dec
    /.fulloptions.noapic dec
    /.fulloptions.nolapic dec
    /.fulloptions.edd_on dec
    /.fulloptions.nodmraid dec
    /.fulloptions.nomodeset dec
    /.fulloptions.free dec
    xmenu .xm_current .fulloptions.acpi_off put
  } {
    xmenu .xm_current .fulloptions.expert put
  } ifelse

  xmenu .xm_list [
    is_live not { /txt_expert_mode } if
    /txt_acpi_off /txt_noapic /txt_nolapic /txt_edd_on /txt_nodmraid
    /txt_nomodeset
    /txt_option_free
  ] put

  xmenu .xm_title /txt_other_options put
  xmenu .xm_check 1 put
  xmenu .xm_allselected xmenu .xm_list get length array put
  0 1 xmenu .xm_list get length 1 sub {
    xmenu .xm_allselected get exch false put
  } for

  pmenu.init
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Change boot options status.
%
% ( ) ==> ( )
%
/fulloptions.change {
  /xmenu xmenu.fulloptions def

  /fulloptions.option 0 string def

  xmenu .xm_allselected get
  0 1 xmenu .xm_allselected get length 1 sub {
    over over get {
      .undef
      is_live not { over .fulloptions.expert eq { pop "priority=low" } if } if
      over .fulloptions.acpi_off eq { pop "acpi=off" } if
      over .fulloptions.noapic eq { pop "noapic" } if
      over .fulloptions.nolapic eq { pop "nolapic" } if
      over .fulloptions.edd_on eq { pop "edd=on" } if
      over .fulloptions.nodmraid eq { pop "nodmraid" } if
      over .fulloptions.nomodeset eq { pop "nomodeset" } if
      over .fulloptions.free eq { pop "apt-setup/restricted=false apt-setup/multiverse=false" } if
      dup {
	% append new option string
	dup length fulloptions.option length add 1 add string
	/fulloptions.newoption exch def
	dup fulloptions.option "%s %s" fulloptions.newoption sprintf
	fulloptions.option free
	/fulloptions.option fulloptions.newoption def
      } if
      pop
    } if
    pop
  } for
  pop

  fulloptions.option length {
    /fulloptions.option inc
  } {
    /fulloptions.option .undef def
  } ifelse
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Update boot options status.
%
% ( ) ==> ( )
%
/fulloptions.update {
  fulloptions.change
  /window.action actRedrawPanel def
  pmenu.update
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show window.
%
% ( ) => ( )
%
/panel.fulloptions {
  "fulloptions" help.setcontext

  window.xmenu
  dup .xmenu xmenu.fulloptions put
  dup .xmenu.change /fulloptions.change put
  dup .xmenu.update /fulloptions.update put
  dup window.init
      window.show
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Return width of panel entry.
%
% ( ) => ( width )
%
/panel.fulloptions.width {
  /xmenu xmenu.fulloptions def

  pmenu.width
} def


% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Redraw panel entry.
%
% ( panel ) => ( )
%
/panel.fulloptions.update {
  /xmenu xmenu.fulloptions def

  pmenu.panel.update
} def
