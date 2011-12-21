#!/usr/bin/python

import sqlite3
import os.path

conn = sqlite3.connect(os.path.expanduser('~/.kde/share/apps/activitymanager/activityranking/database'))

c = conn.cursor()
c.execute('select activity, SUM(s00), SUM(s01), SUM(s02), SUM(s03), SUM(s04), SUM(s05), SUM(s06), SUM(s07), SUM(s10), SUM(s11), SUM(s12), SUM(s13), SUM(s14), SUM(s15), SUM(s16), SUM(s17), SUM(s20), SUM(s21), SUM(s22), SUM(s23), SUM(s24), SUM(s25), SUM(s26), SUM(s27), SUM(s30), SUM(s31), SUM(s32), SUM(s33), SUM(s34), SUM(s35), SUM(s36), SUM(s37), SUM(s40), SUM(s41), SUM(s42), SUM(s43), SUM(s44), SUM(s45), SUM(s46), SUM(s47), SUM(s50), SUM(s51), SUM(s52), SUM(s53), SUM(s54), SUM(s55), SUM(s56), SUM(s57), SUM(s60), SUM(s61), SUM(s62), SUM(s63), SUM(s64), SUM(s65), SUM(s66), SUM(s67) from WeekScores group by activity')

activity = 0

for row in c:
    i = 0

    print "%", row[0]
    print "Y", activity, "= [",
    activity = activity + 1

    for val in row:
        if i == 0:
            i = 1
        else:
            print val, ",",

    print " 0 ];"

