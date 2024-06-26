import os
import re

src_path = os.path.join(os.path.dirname(__file__), "src")
summary_path = os.path.join(src_path, "SUMMARY.md")
references_path = os.path.join(src_path, "references.md")

paths = []
with open(summary_path) as f:
    paths = re.findall(r"\[(.*)\]\((.*)\)", f.read())

links = {}
for name, p in paths:
    with open(os.path.join(src_path, p)) as f:
        local_links = re.findall(r"\[(.*)\]\((.*)\)", f.read())
        links[name] = local_links

with open(references_path, "w") as f:
    for path in links:
        if len(links[path]) == 0:
            continue
        elif path == "References":
            continue
        f.write(f"# {path}\n")

        for i, (name, link) in enumerate(links[path]):
            f.write(f"{i + 1}. {name} [{link}]({link})\n")
