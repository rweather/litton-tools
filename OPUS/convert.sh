#!/bin/sh
echo '/* Generated from opus.drum */'
echo '#if defined(__AVR__)'
echo '#include <avr/pgmspace.h>'
echo 'unsigned char const litton_opus[LITTON_DRUM_MAX_SIZE * 5] PROGMEM = {'
grep -v '^#' opus.drum | awk 'BEGIN{FS=":"}{
print "    [0x" $1 " * 5 + 4] = {0x" substr($2, 1, 2) "},";
print "    [0x" $1 " * 5 + 3] = {0x" substr($2, 3, 2) "},";
print "    [0x" $1 " * 5 + 2] = {0x" substr($2, 5, 2) "},";
print "    [0x" $1 " * 5 + 1] = {0x" substr($2, 7, 2) "},";
print "    [0x" $1 " * 5 + 0] = {0x" substr($2, 9, 2) "},";
}' -
echo '};'
echo '#else'
echo 'static litton_word_t const opus[LITTON_DRUM_MAX_SIZE] = {'
grep -v '^#' opus.drum | awk 'BEGIN{FS=":"}{print "    [0x" $1 "] = 0x" $2 ","}' -
echo '};'
echo '#endif'
