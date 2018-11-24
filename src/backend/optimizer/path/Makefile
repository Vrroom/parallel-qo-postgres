#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for optimizer/path
#
# IDENTIFICATION
#    src/backend/optimizer/path/Makefile
#
#-------------------------------------------------------------------------

CPPFLAGS="$(CPPFLAGS) -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include"
LDFLAGS="$(LDFLAGS) -lglib-2.0" 
subdir = src/backend/optimizer/path
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global

OBJS = allpaths.o clausesel.o costsize.o equivclass.o indxpath.o \
       joinpath.o joinrels.o pathkeys.o tidpath.o

include $(top_srcdir)/src/backend/common.mk
