#!/bin/sh

mbed compile -m K64F --stats-depth=10
python mem_stats.py --limit=20
