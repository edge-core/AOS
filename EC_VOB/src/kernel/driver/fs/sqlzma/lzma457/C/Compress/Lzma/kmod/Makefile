
# Copyright (C) 2006-2007 Junjiro Okajima
# Copyright (C) 2006-2007 Tomas Matejicek, slax.org
#
# LICENSE follows the described one in lzma.txt.

# $Id: Makefile,v 1.1 2007/11/05 05:43:36 jro Exp $

CPPFLAGS += -I$(ACCSRC)/kernel/driver/fs/sqlzma/ -I$(ACCSRC)/kernel/driver/fs/sqlzma/lzma457/C/Compress/Lzma/

#Don't build module, link by linux kernel build process directly
#obj-m += unlzma.o sqlzma.o
obj-y += unlzma.o sqlzma.o
unlzma-y := module.o
sqlzma-y := ../uncomp.o
