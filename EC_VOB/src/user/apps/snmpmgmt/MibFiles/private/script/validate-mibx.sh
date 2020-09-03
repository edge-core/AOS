#! /bin/sh

xmllint --loaddtd -noent -noout --xinclude --schema ../mibx.xsd ../mibx.xml
