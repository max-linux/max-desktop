Section "ServerFlags"
	Option	"AutoEnableDevices"	"false"
	Option	"AutoAddDevices"	"false"
	Option  "DefaultLayout"		"seat%ID_SEAT%"
	Option	"DontZoom"		"true"
	Option	"DontZap"		"true"
	Option	"AllowMouseOpenFail"	"yes"
EndSection


Section "InputDevice"
	Identifier "keyboard%ID_SEAT%"
	Driver	"evdev"
	Option	"CoreKeyboard"
	Option	"Device"	"/dev/usbseat/%ID_SEAT%/keyboard"
	Option	"XkbModel"	"pc105"
	Option	"XkbLayout"	"es"
	Option	"ReopenAttempts" "20"
	Option	"GrabDevice" "true"
EndSection

Section "InputDevice"
	Identifier "mouse%ID_SEAT%"
	Driver	"mouse"
	Option	"CorePointer"
	Option	"Protocol" "ImPS/2"
	Option	"Device"	"/dev/usbseat/%ID_SEAT%/mouse"
	Option	"Buttons" "5"
	Option	"ZAxisMapping" "4 5"
	Option	"ReopenAttempts" "20"
	Option	"GrabDevice" "true"
EndSection

Section "Monitor"
	Identifier   "Generic"
	VendorName   "Monitor Vendor" # value does not matter
	ModelName    "Monitor Model"  # value does not matter
	VertRefresh  50-75
	HorizSync    30-130
EndSection


Section "Device"
        Identifier	"Trigger-plus"
        VendorName	"Magic Control Technology"
	BoardName	"MWS300"
	Driver		"tusb"
	# tusb MULTISEAT driver need VEND_ID and PROD_ID
	Option "DevID" "%VEND_ID%%PROD_ID%"
EndSection


Section "Screen"
	Identifier	"screen%ID_SEAT%"
	Device		"Trigger-plus"
	Monitor		"Generic"
	DefaultDepth 16
	#SubSection "Display"
	#	Depth     16
	#	Modes     "1024x768" "800x600" "640x480"
	#EndSubSection
	#SubSection "Display"
	#	Depth     8
	#	Modes     "1024x768" "800x600" "640x480"
	#EndSubSection
	#SubSection "Display"
	#	Depth     24
	#	Modes     "1024x768" "800x600" "640x480"
	#EndSubSection
EndSection

Section "ServerLayout"
	Identifier "seat%ID_SEAT%"
	Screen	0 "screen%ID_SEAT%" 0 0
	InputDevice "keyboard%ID_SEAT%" "CoreKeyboard"
	#InputDevice "mouse%ID_SEAT%" "CorePointer"
	InputDevice "mouse%ID_SEAT%"
EndSection




