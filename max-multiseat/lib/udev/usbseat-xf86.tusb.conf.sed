Section "ServerFlags"
	Option	"AutoEnableDevices"	"false"
	Option	"AutoAddDevices"	"false"
	Option  "DefaultLayout"		"seat"
	Option	"DontZoom"		"true"
	Option	"DontZap"		"true"
	Option	"AllowMouseOpenFail"	"yes"
EndSection


# ----------- This stuff needs to be adapted to your machine ------------

Section "InputDevice"
	Identifier "keyboard"
	Driver	"evdev"
	Option	"CoreKeyboard"
	Option	"Device"	"/dev/usbseat/%ID_SEAT%/keyboard"
	Option	"XkbModel"	"pc105"
	Option	"XkbLayout"	"es"
	Option	"ReopenAttempts" "60"
	Option	"GrabDevice" "true"
EndSection

Section "InputDevice"
	Identifier "mouse"
	Driver	"mouse"
	Option	"CorePointer"
	Option	"Protocol" "ImPS/2"
	Option	"Device"	"/dev/usbseat/%ID_SEAT%/mouse"
	Option	"Buttons" "5"
	Option	"ZAxisMapping" "4 5"
	Option	"ReopenAttempts" "60"
	Option	"GrabDevice" "true"
EndSection

# ----------------- End of machine specific stuff ------------------------

# ----------------------------------------------------------
# MONITOR section
# ----------------------------------------------------------
# This section contains data for monitor configuration.

# The sisusb driver does not support DDC. 

Section "Monitor"
	Identifier   "Generic"
	VendorName   "Monitor Vendor" # value does not matter
	ModelName    "Monitor Model"  # value does not matter
	VertRefresh  50-75
	HorizSync    30-130
EndSection

# ----------------------------------------------------------
# DEVICE section 
# ----------------------------------------------------------
# This section contains configuration data of the video card.

Section "Device"
        Identifier "SiS USB2VGA"
        VendorName "SiS"   # Value does not matter
	BoardName  "SiS"   # Value does not matter

	Driver     "tusb"

# BusID: Does not matter if you have only one USB2VGA dongle.
# If you have more than one, you can specify the device node
# name or the device number here.
	#BusID     "USB:/dev/sisusbvga0"
	BusID     "USB:/dev/usbseat/%ID_SEAT%/display"

# Please see http://www.winischhofer.at/linuxsisusbvga.shtml for more 
# information

EndSection

# ----------------------------------------------------------
# SCREEN section
# ----------------------------------------------------------
# This section defines the available resulutions and depths.

Section "Screen"
	Identifier "screen"
	Device     "SiS USB2VGA"
	Monitor    "Generic"
	DefaultDepth 16
	SubSection "Display"
		Depth     16
		Modes     "1024x768" "800x600" "640x480"
	EndSubSection
	SubSection "Display"
		Depth     8
		Modes     "1024x768" "800x600" "640x480"
	EndSubSection
	SubSection "Display"
		Depth     24
		Modes     "1024x768" "800x600" "640x480"
	EndSubSection
EndSection

# ----------------------------------------------------------
# Server layout: Combine Monitor, Screen and Device sections
# ----------------------------------------------------------

Section "ServerLayout"
	Identifier "seat"
	Screen	0 "screen" 0 0
	InputDevice "keyboard" "CoreKeyboard"
	InputDevice "mouse" "CorePointer"
EndSection




