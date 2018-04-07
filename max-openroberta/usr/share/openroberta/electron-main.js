const {app, BrowserWindow} = require('electron')
let win

function createWindow () {
	win = new BrowserWindow({
	  webPreferences: {
		nodeIntegration: false,
                icon: '/usr/share/pixmaps/openroberta.png',
	  }
	})

	win.loadURL('http://127.0.0.1:1999')

        win.setMenuBarVisibility(false);
        win.setAutoHideMenuBar(true);
	//win.webContents.openDevTools()

	win.on('closed', () => {
	  win = null
	})

        win.webContents.on("did-fail-load", function() {
          setTimeout(function(){
            console.log("failed to load, retry...");
            win.loadURL('http://127.0.0.1:1999');
          }, 500);
        })

}

app.on('ready', createWindow)

app.on('window-all-closed', () => {
	if (process.platform !== 'darwin') {
	  app.quit()
	}
})

app.on('activate', () => {
	if (win === null) {
	  createWindow()
	}
})

