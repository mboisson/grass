MODULE_TOPDIR = ../..

PGM = r.drain

LIBES = $(VECTORLIB) $(RASTERLIB) $(GISLIB)
DEPENDENCIES = $(VECTORDEP) $(RASTERDEP) $(GISDEP)
EXTRA_INC = $(VECT_INC)
EXTRA_CFLAGS = $(VECT_CFLAGS)

include $(MODULE_TOPDIR)/include/Make/Module.make

default: cmd
