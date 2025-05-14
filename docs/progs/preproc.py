#!/usr/bin/env python3

import json
import sys
import os
import re


def log(x):
    print(x, file=sys.stderr)


CHANGELOG_VERSION = re.compile("^# ([0-9.]+)$")

def parse_json_recursively(json_object, key):
    if type(json_object) is dict and json_object:
        if key == "Chapter":
            content = json_object["content"]
            if json_object["path"] == "ChangeLog.md":
                lines = []
                for line in content.splitlines():
                    lines.append(line)
                    m = CHANGELOG_VERSION.match(line)
                    if m:
                        version = m.groups(0)[0]
                        parts = []
                        prefix = "vastnfs"
                        for part in version.split("."):
                            try:
                                part = int(part)
                            except ValueError:
                                pass
                            parts.append(part)

                        if parts < [4, 0, 4] and parts >= [4]:
                            version = 'v' + version
                        if parts < [4] and parts >= [3]:
                            prefix = "nfsrdma-vastdata"
                        if parts < [3, 9, 10]:
                            continue

                        tarball = prefix + "-" + version + ".tar.xz"
                        url = ("https://vastnfs.vastdata.com/version/" +
                               version + "/source/" + tarball)
                        lines.append("Source: [" + tarball + "](" + url + ")")
                content = "\n".join(lines)
            content += """

---

<small>Document generated on **${GENDATE}** for **${VASTNFS_VERSION}**${VASTNFS_DOCS_SIDE_BRANCH}</small>
"""
            for var in ["VASTNFS_DOCS_SIDE_BRANCH", "VASTNFS_VERSION", "GENDATE"]:
                content = content.replace("${" + var + "}", os.getenv(var, ""))
            json_object["content"] = content

        for key in json_object:
            parse_json_recursively(json_object[key], key)
    elif type(json_object) is list and json_object:
        for item in json_object:
            parse_json_recursively(item, None)

def modify(book):
    parse_json_recursively(book, None)

if __name__ == '__main__':
    if len(sys.argv) > 1: # we check if we received any argument
        if sys.argv[1] == "supports": 
            # then we are good to return an exit status code of 0, since the other argument will just be the renderer's name
            sys.exit(0)

    # load both the context and the book representations from stdin
    context, book = json.load(sys.stdin)
    modify(book)
    # book['sections'][0]['Chapter']['content'] = '# Hello'

    # we are done with the book's modification, we can just print it to stdout, 
    print(json.dumps(book))
