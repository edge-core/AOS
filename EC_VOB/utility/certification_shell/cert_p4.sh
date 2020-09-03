#!/usr/bin/expect
#=================================================================
#  Purpose  : Generate Certificate Automatic-Process shell
#  Author   : Mike Hsieh
#  History  : Init Program at Jan 19, 2012
#  CopyRight: Edge-Core
#=================================================================

set theFile "Certificate.pem"
set rFile [open $theFile r]
set wFile [open "certificate_file.h" w]
set isFirstLine 1
set header      "#define CERTIFICATE_FILE \""
set tail        "\\n\\"

puts $wFile "#ifndef HTTP_CERTIFICATE_FILE_H"
puts $wFile "#define HTTP_CERTIFICATE_FILE_H\n\n"

while {[gets $rFile content] >= 0} {
  split  $content  \n
  if {$isFirstLine==1} {
     set isFirstLine 0
     puts $wFile $header$content$tail
  } else {
     puts $wFile $content$tail
  }
}
puts $wFile "\" \n"
close $rFile

set theFile "private_key.pem"
set rFile [open $theFile r]
#set wFile [open "certificate_file.h" w]
set isFirstLine 1
set header      "#define PRIVATE_KEY_FILE \""
set tail        "\\n\\"

while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa512_1.pvk"
set rFile [open $theFile r]
#set wFile [open "certificate_file.h" w]
set header      "#define PKEY_512B_1 \""
set tail        "\\n\\"

set isFirstLine 1
while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa512_2.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_512B_2 \""
set isFirstLine 1
while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa512_3.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_512B_3 \""
set isFirstLine 1

while {[gets $rFile content] >= 0} {
split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa512_4.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_512B_4 \""
set isFirstLine 1

while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile



set theFile "rsa768_1.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_768B_1 \""
set tail        "\\n\\"
set isFirstLine 1

while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa768_2.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_768B_2 \""
set isFirstLine 1

while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa768_3.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_768B_3 \""
set isFirstLine 1

while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa768_4.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_768B_4 \""
set isFirstLine 1
while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa1024_1.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_1024B_1 \""
set tail        "\\n\\"
set isFirstLine 1

while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa1024_2.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_1024B_2 \""
set isFirstLine 1

while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa1024_3.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_1024B_3 \""
set isFirstLine 1

while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}
puts $wFile "\" \n"
close $rFile

set theFile "rsa1024_4.pvk"
set rFile [open $theFile r]
set header      "#define PKEY_1024B_4 \""
set isFirstLine 1

while {[gets $rFile content] >= 0} {
   split  $content  \n
   if {$isFirstLine==1} {
      set isFirstLine 0
      puts $wFile $header$content$tail
   } else {
      puts $wFile $content$tail
   }
}

puts $wFile "\" \n\n"
puts $wFile "#endif /* HTTP_CERTIFICATE_FILE_H */ \n "
close $rFile

close $wFile
exit