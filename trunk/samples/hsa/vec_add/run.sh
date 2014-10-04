#!/bin/bash

hsailasm --assemble VectorAdd.hsail
m2s --hsa-debug-isa isa.debug VectorAdd.brig
