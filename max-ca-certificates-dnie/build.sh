#!/bin/bash
set -e

mkdir -p build2/ca-certificates-dnie

# Autoridad de Certificación Raíz del DNIe
wget -c 'https://www.dnielectronico.es/ZIP/ACRAIZ-SHA2.CAB' -P build2/
(cd build2 && cabextract ACRAIZ-SHA2.CAB && mv ACRAIZ-SHA2.crt ca-certificates-dnie/AC_RAIZ_DNIE_SHA2.crt && rm -f ACRAIZ-SHA2.CAB)

wget -c 'https://www.dnielectronico.es/ZIP/ACRAIZ-DNIE2.cab' -P build2/
(cd build2 && unzip -u ACRAIZ-DNIE2.cab && mv "AC RAIZ DNIE 2.crt" ca-certificates-dnie/AC_RAIZ_DNIE_2_SHA2.crt && rm -f ACRAIZ-DNIE2.cab)

# Autoridades de Certificación, AC Subordinadas
wget -c 'https://www.dnielectronico.es/ZIP/ACDNIE001-SHA2.crt' -P build2/ && mv build2/ACDNIE001-SHA2.crt build2/ca-certificates-dnie/AC_DNIE_001_SHA2.crt
wget -c 'https://www.dnielectronico.es/ZIP/ACDNIE002-SHA2.crt' -P build2/ && mv build2/ACDNIE002-SHA2.crt build2/ca-certificates-dnie/AC_DNIE_002_SHA2.crt
wget -c 'https://www.dnielectronico.es/ZIP/ACDNIE003-SHA2.crt' -P build2/ && mv build2/ACDNIE003-SHA2.crt build2/ca-certificates-dnie/AC_DNIE_003_SHA2.crt
wget -c 'https://www.dnielectronico.es/ZIP/AC_DNIE_004.crt'    -P build2/ && mv build2/AC_DNIE_004.crt build2/ca-certificates-dnie/AC_DNIE_004_SHA2.crt
wget -c 'https://www.dnielectronico.es/ZIP/AC_DNIE_005.crt'    -P build2/ && mv build2/AC_DNIE_005.crt build2/ca-certificates-dnie/AC_DNIE_005_SHA2.crt
wget -c 'https://www.dnielectronico.es/ZIP/AC_DNIE_006.crt'    -P build2/ && mv build2/AC_DNIE_006.crt build2/ca-certificates-dnie/AC_DNIE_006_SHA2.crt

# Autoridad de Validación AV DNIE FNMT
wget -c 'https://www.dnielectronico.es/descargas/certificados/Ocsp_Responder_AV_DNIE_FNMT_2017_SHA-2.zip' -P build2/
(cd build2 && unzip -u Ocsp_Responder_AV_DNIE_FNMT_2017_SHA-2.zip && mv VADNI2_2017.cer ca-certificates-dnie/AV_DNIE_FNMT_SHA2.cer && rm Ocsp_Responder_AV_DNIE_FNMT_2017_SHA-2.zip)

# check SHA256
sha256sum -c ./ca-certificates-dnie.sha256
