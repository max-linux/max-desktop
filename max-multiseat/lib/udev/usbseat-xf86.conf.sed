Section "ServerFlags"
	Option	"AutoEnableDevices"	"false"
	Option	"AutoAddDevices"	"false"
	Option  "DefaultLayout"		"seat"
	Option	"DontZoom"		"true"
	Option	"DontZap"		"true"
	Option	"AllowMouseOpenFail"	"yes"
	Option	"DontVTSwitch"
EndSection

Section "Module"
	Load "ddc"
EndSection

Section "Files"                                                                                                              
    ModulePath      "/usr/lib/xorg/modules"
    ModulePath      "/usr/local/lib/xorg/modules"
EndSection

Section "Device"
	Identifier "dl"
	Driver	   "displaylink"
#	Option "rotate"	"CCW"
	Option "fbdev"	"/dev/usbseat/%ID_SEAT%/display"
EndSection

Section "InputDevice"
	Identifier "keyboard"
	Driver	"evdev"
	Option	"CoreKeyboard"
	Option	"Device"	"/dev/usbseat/%ID_SEAT%/keyboard"
	Option	"XkbModel"	"pc105"
	Option	"XkbLayout"	"es"
EndSection

Section "InputDevice"
	Identifier "mouse"
	Driver	"mouse"
	Option	"CorePointer"
	Option	"Protocol" "ImPS/2"
	Option	"Device"	"/dev/usbseat/%ID_SEAT%/mouse"
	Option	"Buttons" "5"
	Option	"ZAxisMapping" "4 5"
EndSection

Section "Monitor"
	Identifier "monitor"
EndSection

Section "Screen"
	Identifier "screen"
	Device "dl"
	Monitor "monitor"
EndSection

Section "ServerLayout"
	Identifier "seat"
	Screen	0 "screen" 0 0
	InputDevice "keyboard" "CoreKeyboard"
	InputDevice "mouse" "CorePointer"
EndSection
	

