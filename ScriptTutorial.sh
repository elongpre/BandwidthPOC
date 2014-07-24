#!/bin/bash
# My first script

echo "Hello World!"

if [ "$1" != "" ]; then
    echo "Positional parameter 1 contains something"
    curl -s http://rss.accuweather.com/rss/liveweather_rss.asp\?metric\=${METRIC}\&locCode\=$1 | perl -ne 'if (/Currently/) {chomp;/\<title\>Currently: (.*)?\<\/title\>/; print "$1 \n"; }'
else
    echo "Positional parameter 1 is empty"
fi
