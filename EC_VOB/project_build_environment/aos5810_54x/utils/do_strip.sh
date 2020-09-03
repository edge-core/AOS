#!/bin/sh

if [ -d ${ACCROOTFS}/usr/lib ]; then
	cd ${ACCROOTFS}/usr/lib
	ls *.so.0.0.0 | xargs ${CROSS_COMPILE}strip
fi

if [ -d ${ACCROOTFS}/usr/bin ]; then
	cd ${ACCROOTFS}/usr/bin
	find . -type f | xargs ${CROSS_COMPILE}strip
fi

if [ -d  ${ACCROOTFS}/usr/sbin ]; then
	cd ${ACCROOTFS}/usr/sbin
	ls sysinit_proc | xargs ${CROSS_COMPILE}strip
fi

exit 0
