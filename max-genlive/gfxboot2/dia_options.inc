% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
%
% Panel 'more options' entry.
%
% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

% - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% Show help window.
%
% ( ) => ( )
%
/panel.options {
  /panel panel.full def

  boot.show not {
    /boot.show true def
    boot.setup

    boot.window .color.fg get setcolor
    boot.window .ed.font get setfont  

    boot.ed 3 get
    boot.ed over edit.init
    "" ne { boot.ed ' ' edit.input } if
  } if

  main.redraw

  panel.fulloptions
} def
