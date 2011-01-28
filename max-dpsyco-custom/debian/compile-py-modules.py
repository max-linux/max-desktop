import os, sys
import py_compile


def compile (arg, dirname, files):
    #print "COMPILE: arg=%s dirname=%s files=%s" %(arg, dirname, files)
    for file in files:
        if file.endswith(arg):
            fileabs=os.path.abspath(os.path.join(dirname, file))
            try:
                os.stat(fileabs)
            except:
                continue
            print "Found python ==> %s"%(fileabs)
            py_compile.compile(fileabs)

def clean (arg, dirname, files):
    #print "CLEAN: arg=%s dirname=%s files=%s" %(arg, dirname, files)
    for file in files:
        if file.endswith(arg):
            fileabs=os.path.abspath(os.path.join(dirname, file))
            print "Found python compiled ==> %s"%(fileabs)
            os.unlink(fileabs)

curdir=os.path.join(os.path.curdir, "debian/max-dpsyco-custom")

if len(sys.argv) == 1:
    print "Need and argument: --compile or --clean"
    sys.exit(1)

if sys.argv[1] == "--compile":
    os.path.walk( curdir, compile, ".py")

if sys.argv[1] == "--clean":
    os.path.walk( curdir, clean, ".pyc")

