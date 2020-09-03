#!/bin/sh
#=================================================================
#  Purpose  : Generate Certificate Automatic-Process shell
#  Author   : Mike Hsieh
#  History  : Init Program at Jan 19, 2012
#  CopyRight: Edge-Core
#=================================================================
echo "Read config parameter for certificate process***"
grep C= cert_config.txt | sed 's/C=//g' > Country
grep ST= cert_config.txt | sed 's/ST=//g' > State
grep L= cert_config.txt | sed 's/L=//g' > Local
grep O= cert_config.txt | sed 's/O=//g' > Organization
grep OU= cert_config.txt | sed 's/OU=//g' > Organization_Unit
grep CN= cert_config.txt | sed 's/CN=//g' > Comman_Name
grep emailAddress= cert_config.txt | sed 's/emailAddress=//g' > Email_Address
grep passphrase= cert_config.txt | sed 's/passphrase=//g' > PassPhrase
grep pkeypasswd= cert_config.txt | sed 's/pkeypasswd=//g' > PkeyPasswd
