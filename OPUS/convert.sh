#!/bin/sh
echo '/* Generated from opus.drum */'
echo 'static litton_word_t const opus[LITTON_DRUM_MAX_SIZE] = {'
grep -v '^#' opus.drum | awk 'BEGIN{FS=":"}{print "    [0x" $1 "] = 0x" $2 ","}' -
echo '};'
