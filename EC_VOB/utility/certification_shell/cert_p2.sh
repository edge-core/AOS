#!/usr/bin/expect
#=================================================================
#  Purpose  : Generate Certificate Automatic-Process shell
#  Author   : Mike Hsieh
#  History  : Init Program at Jan 19, 2012
#  CopyRight: Edge-Core
#=================================================================


#=================================================================
# Procedure: read customer information
#=================================================================
set rFile [open "Country" r]
while {[gets $rFile content] >= 0} {
set certInfo_Country $content
}
close $rFile

set rFile [open "State" r]
while {[gets $rFile content] >= 0} {
set certInfo_State $content
}
close $rFile

set rFile [open "Local" r]
while {[gets $rFile content] >= 0} {
set certInfo_Local $content
}
close $rFile

set rFile [open "Organization" r]
while {[gets $rFile content] >= 0} {
set certInfo_Organization $content
}
close $rFile

set rFile [open "Organization_Unit" r]
while {[gets $rFile content] >= 0} {
set certInfo_Organization_Unit $content
}
close $rFile

set rFile [open "Comman_Name" r]
while {[gets $rFile content] >= 0} {
set certInfo_Comman_Name $content
}
close $rFile

set rFile [open "Email_Address" r]
while {[gets $rFile content] >= 0} {
set certInfo_Email_Address $content
}
close $rFile

set rFile [open "PassPhrase" r]
while {[gets $rFile content] >= 0} {
set certInfo_PassPhrase $content
}
close $rFile

set rFile [open "PkeyPasswd" r]
while {[gets $rFile content] >= 0} {
set certInfo_PkeyPasswd $content
}
close $rFile

#=================================================================
# Procedure: Generate config file for key process
#=================================================================
set certConfigInfo "/C=$certInfo_Country/ST=$certInfo_State/L=$certInfo_Local/O=$certInfo_Organization/OU=$certInfo_Organization_Unit/CN=$certInfo_Comman_Name/emailAddress=$certInfo_Email_Address"

set wFile [open "certInfo.txt" w]
puts $wFile $certConfigInfo
close $wFile

set wFile [open "passphrase.txt" w]
puts $wFile $certInfo_PassPhrase
close $wFile

set wFile [open "pkeypasswd.txt" w]
puts $wFile $certInfo_PkeyPasswd
close $wFile

#=================================================================
# Procedure: Generate pass_phrase_file.h
#=================================================================
set wFile [open "pass_phrase_file.h" w]
puts $wFile "#ifndef HTTPS_PASS_PHRASE_FILE_H"
puts $wFile "#define HTTPS_PASS_PHRASE_FILE_H\n\n\n"

puts $wFile "#ifndef PASS_PHRASE"
set header  "#define PASS_PHRASE \""
set tailer  "\""
puts $wFile $header$certInfo_PassPhrase$tailer
puts $wFile "#endif /*PASS_PHRASE*/\n\n\n"

puts $wFile "#ifndef PKEY_PASSWD"
set header  "#define PKEY_PASSWD \""
set tailer  "\""
puts $wFile $header$certInfo_PkeyPasswd$tailer
puts $wFile "#endif /*PKEY_PASSWD*/\n\n\n"

puts $wFile "#ifndef CA_PASS_PHRASE"
set header  "#define CA_PASS_PHRASE  "
set passwd  "\"1234\""
puts $wFile $header$passwd
puts $wFile "#endif /*CA_PASS_PHRASE*/\n\n\n"

puts $wFile "#endif /* HTTPS_PASS_PHRASE_FILE_H */"
close $wFile

exit


