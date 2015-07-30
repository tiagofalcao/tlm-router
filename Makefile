#!/usr/bin/make -f
MKFILE := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOTDIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE))))
BUILDDIR ?= build

.PHONY:		default
default:	all

PACKAGE := tlm-router
DESCRIPTION := A router in TLM2 SystemC with 4 directions (NSEW)
VERSION := 0.1

include gnudirs.mk

SHELL = /bin/sh
CPP ?= cpp
CPPFLAGS ?=
CC ?= gcc
CXX ?= g++
AR ?= ar
CFLAGS ?= -O3 -fPIC
CXXFLAGS ?= -O3 -std=c++11 -fPIC
LDFLAGS ?=
LDLIBS ?=
ARFLAGS ?= rcs

REQUIREMENTS := systemc
OPTIONALS := 

include rules.mk
