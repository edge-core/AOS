#=================================================================
#  Purpose  : Generate Certificate Automatic-Process shell
#  Author   : Mike Hsieh
#  History  : Init Program at Jan 19, 2012
#  CopyRight: Edge-Core
#=================================================================

dos2unix cert_p1.sh
chmod +x cert_p1.sh
dos2unix cert_p2.sh
chmod +x cert_p2.sh
dos2unix cert_p3.sh
chmod +x cert_p3.sh
dos2unix cert_p4.sh
chmod +x cert_p4.sh

echo "Certificate and PassPhrase process ..."
./cert_p1.sh
echo "Generate pass_phrase_file.h"
./cert_p2.sh
echo "Execute certificate command: openssl ... "
./cert_p3.sh
echo "Generate certificate_file.h ***\n"
./cert_p4.sh
echo "Done ..."

rm -rf *.pvk
# rm -rf *.pem
rm -rf passphrase.txt  pkeypasswd.txt certInfo.txt
rm -rf Country  State Local Email_Address  PkeyPasswd PassPhrase Organization  Organization_Unit Comman_Name