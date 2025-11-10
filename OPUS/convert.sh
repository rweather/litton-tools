#!/bin/sh
echo '/* Generated from opus.drum */'
echo '#if defined(__AVR__)'
echo '#include <avr/pgmspace.h>'
echo '#elif !defined(PROGMEM)'
echo '#define PROGMEM'
echo '#endif'
echo 'static litton_word_t const opus[LITTON_DRUM_MAX_SIZE] PROGMEM = {'
grep -v '^#' opus.drum | awk 'BEGIN{FS=":"}{print "    [0x" $1 "] = 0x" $2 ","}' -
echo '};'
