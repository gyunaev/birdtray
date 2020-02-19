#!/usr/bin/env python3
import os
import re

from string import hexdigits
from argparse import ArgumentParser, OPTIONAL

HEADER_PATTERN = r' {2}.*\)>'
USER_DATA_PATTERN = r'=([\s\S]*?)(?<!\\)\)'
ALLOWED_USER_DATA_CHARS = hexdigits + '^'


def anonymize(path):
    outputPath = path + '-anonymize.msf'
    if os.path.exists(outputPath) and input('The output path "{path}" already exists. Overwrite? '
                                            '[yN] '.format(path=outputPath)).lower() != 'y':
        return
    with open(path) as file:
        content = file.read()
    headerMatch = re.search(HEADER_PATTERN, content)
    if headerMatch is not None:
        endIndex = headerMatch.end()
        header = content[:endIndex]
        content = content[endIndex:]
    else:
        header = ''
    for match in sorted(filter(lambda x: not all(char in ALLOWED_USER_DATA_CHARS for char in x),
                               re.findall(USER_DATA_PATTERN, content)), key=len, reverse=True):
        content = content.replace(match, '\\\n'.join('X' * len(x) for x in match.split('\\\n')))
    with open(outputPath, 'w') as file:
        file.write(header + content)
    print('Success')
    print('Make sure that all personal information was replaced before uploading the mork file!')


if __name__ == '__main__':
    parser = ArgumentParser(
        description='Replaces all personal data in a mork file with X characters')
    parser.add_argument('morkPath', nargs=OPTIONAL, default=None,
                        help='The path to the mork file to anonymize')
    morkPath = parser.parse_args().morkPath
    if morkPath is None:
        morkPath = input('Path to the mork file: ')
    anonymize(morkPath)
