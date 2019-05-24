# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000, 2001, 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2008
# Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>

ifdef CONFIG_SPL_BUILD
obj-y	= spl_build.o
hostprogs-y := tools/mkariesspl
always := $(hostprogs-y)
else
obj-y	+= aries.o
obj-y	+= onenand.o
obj-y	+= board_detect.o
endif
