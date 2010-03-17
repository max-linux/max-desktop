/*
Javascript component of ubiquity-slideshow global to all variations.

* Interprets parameters passed via location.hash (in #?param1=key?param2 format)
* Creates an animated slideshow inside the #slideshow element.
* Automatically loads a requested locale, based on the default slides.
* Manages slideshow controls, if requested via parameters.

Assumptions are made about the design of the html document this is inside of.
Please see slides/ubuntu/index.html for an example of this script in use.


Dependencies (please load these first):
link/prototype.js
link/effects.js, link/fastinit.js, link/crossfade.js
directory.js (note that this file does not exist yet, but will when the build script runs)
*/

/* TODO: Accept extra parameters from host HTML file!
         (Perhaps variables defined ahead of time)
*/

window.onDomReady = DomReady;
function DomReady(fn)
{
	document.addEventListener("DOMContentLoaded", fn, false);
}


var options = []; /* this will hold parameters passed to the slideshow */
var slideshow;

window.onDomReady(function(){
	parameters = window.location.hash.slice(window.location.hash.indexOf('#') + 1).split('?');
	
	for(var i = 0; i < parameters.length; i++)
	{
		hash = parameters[i].split('=');
		options.push(hash[0]);
		options[hash[0]] = hash[1];
	}
	
	if ( options.indexOf('locale') > -1 )
		setLocale(options['locale']);
	
	if ( options.indexOf('rtl') > -1 )
		loadRTL();
	
	
	Crossfade.setup({autoLoad:false, random:false, interval:45, duration:0.5, loop:false, transition:Crossfade.Transition.Cover });
	
	slideshow = new Crossfade('slideshow');
	
	if ( options.indexOf('controls') > -1 ) {
		slideshow.options.loop = true;
		$('debug-controls').style.display = "block";
		$('current-slide').value = slideshow.filenames[0];
		$('prev-slide').onclick = prevSlide;
		$('next-slide').onclick = nextSlide;
		//slideshow.stop();
	};
});


function setLocale(locale) {
	var slideanchors = $$("div#slideshow div a");
	
	slideanchors.each(function(anchor) {
		var slide_name = anchor.readAttribute("href");
		var translated_url = "./loc."+locale+"/"+slide_name;
		
		try {
			if ( directory[locale][slide_name] == true )
				anchor.href = translated_url;
		}
		catch(err) {
			/*
			This usually happens if the directory object
			(auto-generated at build time, placed in ./directory.js)
			does not exist. That object is needed to know whether
			a translation exists for the passed locale.
			*/
		}
	})
}

function loadRTL() {
	/* Note: Once you turn on RTL, there is no going back! */
	var fileref=document.createElement("link");
	fileref.setAttribute("rel", "stylesheet");
	fileref.setAttribute("type", "text/css");
	fileref.setAttribute("href", "link/rtl.css");
	document.getElementsByTagName("head")[0].appendChild(fileref)
}

function nextSlide() {
	slideshow.next();
}

function prevSlide() {
	slideshow.previous();
}
