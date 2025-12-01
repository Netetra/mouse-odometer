#!/bin/bash

# Config
xram_size=0x1800
xram_loc=0x0000
code_size=0xF000

# Clean
bash ./scripts/clean.sh

set -eu -o pipefail

# Compile
find ./src -name "*.c" | xargs -I{} sdcc -I ./include -c -V -mmcs51 --stack-auto --model-large --xram-size $xram_size --xram-loc $xram_loc --code-size $code_size -o out/ {}
sdcc ./out/*.rel -I ./include -V -mmcs51 --stack-auto --model-large --xram-size $xram_size --xram-loc $xram_loc --code-size $code_size -o ./out/main.ihx

# Convert to hex file
packihx ./out/main.ihx > ./out/main.hex

# Complete!
