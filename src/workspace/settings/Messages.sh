#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui` >> rc.cpp
$XGETTEXT *.cpp -o $podir/kcm_activities.pot
