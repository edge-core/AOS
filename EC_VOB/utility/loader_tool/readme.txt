-----------------------
README for loader tools
-----------------------

encrypt_utility
---------------
encrypt_utility is a utility used to generate the encrypted text from the given
plain text. The encryption algorithm of this utility is exactly the same with
the one used in the loader(i.e. uboot). When a loader backdoor is defined in
the loader source code, it is required to use encrypted text in the source code.
This utility can be used to generate the encrypted text to be defined in the
loader source code.

encrypt_utility command syntax:
encrypt [options] password_text
Options:
  -h     Help output
'password_text'   is your password

Example:
encrypt_utility j5aa0zid
Encrypted Password: 9rzzmo83


passwdgenerate
--------------
passwdgenerate is a utility to generate a random string to be used as password
according to the given arguments "project id" and "project name". Note that the
output string by this utility is in clear text format. It is required to be
converted to encrypted text format by encrypt_utility.

passwdgenerate command syntax:
passwdgenerate [options] project_id project_name
Options:
 -h     Help output
'project_id'   is your project id
'project_name' is your project name

Example:
passwdgenerate 319 es4626f_flf_38
Password Generate: 'wkwaibnn'

When specify project id, please type the number in decimal.
When specify project name, please type all alphabets in lower case.

When generate a password for "enable" command in uboot, append "_priv" to the
end of the "real project name" for the argument "project_name". For example,
to generate the password for "enable" command for the project es4626f_flf_38,
execute the command "passwdgenerate 319 es4626f_flf_38_priv".
