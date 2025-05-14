#!/usr/bin/env python3

import json
import sys
import os
import lxml.html
import lxml.etree as etree
import shutil

def log(x):
    print(x, file=sys.stderr)

def html_fix(path, root):
    purecss = etree.fromstring("""<link rel="stylesheet" href="https://unpkg.com/purecss@2.1.0/build/pure-min.css" integrity="sha384-yHIFVG6ClnONEA5yB5DJXfW2/KC173DIQrYoZMEtBvGzmf0PKiGyNEqe9N6BNDBH" crossorigin="anonymous" />""")

    comps = path[len(root)+1:]
    comps = comps.split("/")
    rel = "/".join([".." for x in comps])
    if len(comps) == 1:
        rel = ""
    else:
        rel = "/".join([".." for x in comps[:-1]]) + "/"

    othercss = etree.fromstring(f"""<link rel="stylesheet" href="{rel}css/custom.css" />""")
    otherjs = etree.fromstring(f"""<script src="{rel}js/custom.js" />""")

    topbar = open(os.path.join(os.path.dirname(__file__), "html", "topbar.html")).read()
    topbar = topbar.replace("${DOCS_VERSION}", os.getenv("DOCS_VERSION"))
    topbar = lxml.html.fromstring(topbar)

    html = lxml.html.fromstring(open(path).read())
    head = html.find("head")
    head.append(purecss)
    head.append(othercss)

    head = html.find("body")
    head.append(otherjs)

    head = html.xpath("//div[@class='left-buttons']")
    head[0].insert(2, topbar)

    open(path, "wb").write(lxml.html.tostring(html))

def main():
    render_context = json.load(sys.stdin)
    dest = render_context["destination"]
    html = os.path.join(os.path.dirname(dest), "html")

    for custom in ["css", "js"]:
        src = os.path.join(render_context["root"], "progs", custom)
        dest = os.path.join(html, custom)
        cmd = f"mkdir -p {dest} && cp -a {src}/* {dest}/"
        os.system(cmd)

        for file in os.listdir(src):
            fulldest = os.path.join(dest, file)
            if not os.path.isfile(fulldest):
                continue
            content = open(fulldest).read()
            content = content.replace("${DOCS_VERSION}", os.getenv("DOCS_VERSION"))
            open(fulldest, "w").write(content)

    for base, dirs, files in os.walk(html, topdown=False):
        for name in files:
            if name.endswith(".html"):
                html_fix(os.path.join(base, name), html)

if __name__ == '__main__':
    main()
