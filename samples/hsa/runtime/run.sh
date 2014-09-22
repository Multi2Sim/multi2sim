#!/bin/bash

hsailasm --assemble runtime.hsail
m2s --hsa-debug-isa isa.debug --hsa-debug-driver driver.debug --hsa-debug-loader loader.debug runtime.brig
