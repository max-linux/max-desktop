all:
	#none

clean:
	rm -f *.deb *.changes *.tar.gz *.diff.gz *.dsc *.build *.asc
	find -name *~ | xargs rm -f 
