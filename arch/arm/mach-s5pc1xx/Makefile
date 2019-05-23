# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2008
# Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>

obj-y	= cache.o

ifdef CONFIG_SPL_BUILD
obj-y	+= clock_init.o
obj-y	+= dmc_init.o
obj-y	+= lowlevel_init.o
obj-y	+= power.o
obj-y	+= spl_build.o
obj-y	+= tzpc.o
else
obj-y	+= reset.o

obj-y	+= clock.o
obj-y	+= system.o
endif
