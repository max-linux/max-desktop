#!/bin/sh

set -e

# include config
. /root/.firefox.secrets


rm -f maxubufox.xpi
bash build.sh

jpm sign --api-key "$JWT_ISSUER" --api-secret "$JWT_SECRET" --xpi maxubufox.xpi

rm maxubufox.xpi
