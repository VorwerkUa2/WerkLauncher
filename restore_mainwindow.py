
import re

walkthrough_path = r'C:\Users\knyaz\.gemini\antigravity\brain\079b1030-dc40-40e8-9f03-7ef42e238a3c\walkthrough.md.resolved.1'
output_path = r'c:\Users\knyaz\Desktop\MCL\launcher\ui\MainWindow.cpp'

with open(walkthrough_path, 'r', encoding='utf-8') as f:
    lines = f.readlines()

# MainWindow.cpp Version 2 is between lines 1930 and 3839 (1-indexed)
# In 0-indexed list, that's 1929 to 3838
start = 1929
end = 3839

content = []
for i in range(start, end):
    line = lines[i]
    # Remove line number prefix "123: "
    match = re.match(r'^\d+: (.*)', line)
    if match:
        content.append(match.group(1))
    else:
        # Fallback if line number is missing for some reason
        content.append(line)

with open(output_path, 'w', encoding='utf-8') as f:
    f.write('\n'.join(content))

print(f"Restored {len(content)} lines to {output_path}")
