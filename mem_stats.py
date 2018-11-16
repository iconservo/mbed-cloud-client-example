#!/usr/bin/env python

import json
from pprint import pprint
from prettytable import PrettyTable, HEADER
import argparse

print_sections = ['size_flash', 'size_ram'];

def generate_table(stats):
    """Generate a table from a memoy map

    Returns: string of the generated table
    """
    # Create table
    columns = ['Module']
    columns.extend(print_sections)

    table = PrettyTable(columns, junction_char="|", hrules=HEADER)
    table.align["Module"] = "l"
    for col in print_sections:
        table.align[col] = 'r'

    for i in stats:
        row = [i['module']]
        for k in print_sections:
            row.append("{}".format(i[k]))                       
        table.add_row(row)

    output = table.get_string()
    output += '\n'
        
    return output

if __name__ == '__main__':
    # Parse Options
    parser = argparse.ArgumentParser(description='Static RAM usage helper tool')
    parser.add_argument('--limit', type=int, default=0, help='modules count limit (e.g. --limit=10 will print top 10 by RAM usage)')
    args = parser.parse_args()

    with open('BUILD/K64F/GCC_ARM/mbed-cloud-client-example_map.json') as f:
        stats = json.load(f)

    stats = [ { 'module': i['module'], 'size_flash': i['size']['.text'], 'size_ram': i['size']['.bss'] + i['size']['.data'] } for i in stats if 'size' in i ]
    stats = [ i for i in stats if i['size_ram'] > 0 ]
    # Sort descending by static RAM usage 
    stats = sorted(stats, key=lambda v: v['size_ram'], reverse=True)

    if args.limit != 0:
        stats = stats[:args.limit]
        
    # pprint(data)
    print(generate_table(stats))