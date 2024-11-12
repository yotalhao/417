# Extension name and modules to compile
EXTENSION = dna
MODULES = dna kmer qkmer

# SQL files and control file for the extension
DATA = dna--1.0.sql dna.control

# Using pg_config to find PostgreSQL paths
PG_CONFIG ?= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# Compilation options
CFLAGS += -Wall -Werror -O2 -fPIC
