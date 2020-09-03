all: addIB

addIB:
	gcc -o $(ACCUTIL)/loader_tool/encrypt_utility $(ACCUTIL)/loader_tool/encrypt_utility.c $(BOOTLOADER_DIR)/common/encrypt.c
	gcc -o $(ACCUTIL)/loader_tool/passwdgenerate $(ACCUTIL)/loader_tool/passwdgenerate.c

clean:
	rm $(ACCUTIL)/loader_tool/encrypt_utility
	rm $(ACCUTIL)/loader_tool/passwdgenerate
