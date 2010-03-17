#!/bin/sh

cd $(dirname $0);
gimp -i -f -d -b - < reflection-script.scm;
