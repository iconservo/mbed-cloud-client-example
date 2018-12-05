#!/bin/sh

mbed compile -m NRF52840_DK --stats-depth=10
# python mem_stats.py --limit=20
