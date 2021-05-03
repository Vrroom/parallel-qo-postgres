#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for the parallel query optimizer module
#
# Copyright (c) 2021, Sumit Chaturvedi
#
# src/backend/optimizer/parallel/Makefile
#
#-------------------------------------------------------------------------

subdir = src/backend/optimizer/parallel
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global

OBJS = parallel_main.o parallel_utils.o parallel_worker.o \
	   parallel_eval.o parallel_tree.o parallel_list.o

include $(top_srcdir)/src/backend/common.mk
