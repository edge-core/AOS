#!/usr/bin/expect
#=================================================================
#  Purpose  : Generate Certificate Automatic-Process shell
#  Author   : Mike Hsieh
#  History  : Init Program at Jan 19, 2012
#  CopyRight: Edge-Core
#=================================================================
set rFile [open "passphrase.txt" r]
while {[gets $rFile content] >= 0} {
set passphrase $content
}
close $rFile

set rFile [open "pkeypasswd.txt" r]
while {[gets $rFile content] >= 0} {
set pkeypasswd $content
}
close $rFile

set rFile [open "certInfo.txt" r]
while {[gets $rFile content] >= 0} {
set certInfo $content
}
close $rFile

spawn openssl req -new -x509 -keyout private_key.pem -out Certificate.pem -days 3650 -subj "$certInfo"

expect "Enter PEM pass phrase:"
send "$passphrase\r"
expect "Verifying - Enter PEM pass phrase:"
send "$passphrase\r"
expect eof

spawn openssl genrsa -out rsa512_1.pvk -des3 512

expect "Enter pass phrase for rsa512_1.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa512_1.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa512_2.pvk -des3 512

expect "Enter pass phrase for rsa512_2.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa512_2.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa512_3.pvk -des3 512

expect "Enter pass phrase for rsa512_3.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa512_3.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa512_4.pvk -des3 512

expect "Enter pass phrase for rsa512_4.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa512_4.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa768_1.pvk -des3 768

expect "Enter pass phrase for rsa768_1.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa768_1.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa768_2.pvk -des3 768

expect "Enter pass phrase for rsa768_2.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa768_2.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa768_3.pvk -des3 768

expect "Enter pass phrase for rsa768_3.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa768_3.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa768_4.pvk -des3 768

expect "Enter pass phrase for rsa768_4.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa768_4.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa1024_1.pvk -des3 1024

expect "Enter pass phrase for rsa1024_1.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa1024_1.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa1024_2.pvk -des3 1024

expect "Enter pass phrase for rsa1024_2.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa1024_2.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa1024_3.pvk -des3 1024

expect "Enter pass phrase for rsa1024_3.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa1024_3.pvk:"
send "$pkeypasswd\r"
expect eof

spawn openssl genrsa -out rsa1024_4.pvk -des3 1024

expect "Enter pass phrase for rsa1024_4.pvk:"
send "$pkeypasswd\r"
expect "Verifying - Enter pass phrase for rsa1024_4.pvk:"
send "$pkeypasswd\r"
expect eof

exit