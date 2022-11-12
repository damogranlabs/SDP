import os
import sys
import time as systime
import timeit

"""
"terminal.external.windowsExec": "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe",
"terminal.integrated.shell.windows": "C:\\WINDOWS\\System32\\WindowsPowerShell\\v1.0\\powershell.exe",
"""
# & C:/Python27/python.exe "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/timeit_test.py"
# & "C:/Program Files (x86)/Python36-32/python.exe" "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/timeit_test.py"


def to_time_it():

    print("what the fuck is this?!?")


AVERAGE = 10000
take = 0
avg = 0
max_time = 0

print("\nResponse timing test start:")

while take <= AVERAGE:
    take = take + 1

    tajm = timeit.timeit(to_time_it, number=1) * 1000

    avg = avg + tajm
    if tajm > max_time:
        max_time = tajm

    print("\t%s Execution time: %.2f ms" % (take - 1, tajm))

print("OVER")

avg_time = avg / AVERAGE
print("\n\tAverage time: %.2f ms" % avg_time)
print("\tMax time: %.2f ms" % max_time)