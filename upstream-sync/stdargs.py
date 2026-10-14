#!/usr/bin/env python3
"""Strip or re-add the __stdargs annotations of the amigaos header copies.

  stdargs.py strip <file>   print <file> without CRs and __stdargs
  stdargs.py add <file>     print <file> with __stdargs on declarations

"add" annotates, outside comments:
  - a function declaration line: a return type, then an identifier and
    "(", ending in ";" or "," or continuing on the next line, when the
    line is not a typedef, static/inline definition, statement or macro;
  - a return type alone on its line when the next line starts "name (";
  - a function-pointer declarator "type (*name)(", at any indentation,
    including callback parameters inside a prototype;
  - a function or function-pointer typedef, after "typedef".
__stdargs goes after the indentation and any leading extern/_Noreturn.
Lines that already have __stdargs are left alone, so "add" can run again
on a file that is partly annotated.
"""
import re
import sys

STDARGS = re.compile(r'\b__stdargs[ \t]+|[ \t]+__stdargs\b')
PPGUARD = re.compile(r'^\s*#\s*(ifn?def|define|undef)\s+__stdargs\b')
KEYWORD = r'(?:return|else|if|while|for|do|case|goto|sizeof|switch)\b'
SKIP = re.compile(r'^\s*(#|static\b|_ELIDABLE_INLINE\b|__inline\b|inline\b|'
                  r'_BEGIN_STD_C|_END_STD_C|__BEGIN_DECLS|__END_DECLS|'
                  r'typedef\b|__asm|asm\b|extern\s+"C"|struct\b[^(]*\{|union\b|enum\b|\}|\{|'
                  + KEYWORD + r')')
LEAD = re.compile(r'^(\s*(?:extern\s+)?(?:_Noreturn\s+)?)')
DECL = re.compile(r'^\s*(?:extern\s+)?[A-Za-z_][\w\s\*]*?[\s\*]\w+\s*\(')
TYPEONLY = re.compile(r'^(?:extern\s+)?[A-Za-z_][\w \t\*]*$')
NAMEPAREN = re.compile(r'^\s*\w+\s*\(')
TYPEDEF = re.compile(r'^(\s*typedef\s+)(?=[A-Za-z_][\w\s\*]*?(?:[\s\*]\w+|\(\s*\*\s*\w+\s*\))\s*\()')
# a callback parameter inside a prototype: "(" or "," then "type (*name)("
PARAMFN = re.compile(r'([(,]\s*)((?:const\s+)?[A-Za-z_]\w*(?:\s+\w+)*(?:\s*\*)*\s*)'
                     r'(\(\s*\*\s*\w*\s*\)\s*\()')
FNPTR = re.compile(r'^\s*(?:const\s+)?[A-Za-z_][\w\s\*]*?\(\s*\*\s*\w+\s*\)\s*\(')


def strip(text):
    return '\n'.join(l if PPGUARD.match(l) else STDARGS.sub('', l)
                     for l in text.replace('\r', '').split('\n'))


def head(line):
    lead = LEAD.match(line).group(1)
    return lead + '__stdargs ' + line[len(lead):]


def params(line):
    """Annotate callback parameters after the first "(" of a declaration."""
    i = line.find('(')
    if i < 0:
        return line
    return line[:i] + PARAMFN.sub(
        lambda m: m.group(0) if '__stdargs' in m.group(2)
        else m.group(1) + '__stdargs ' + m.group(2) + m.group(3), line[i:])


def add(text):
    out = []
    lines = text.split('\n')
    incomment = False
    for i, line in enumerate(lines):
        s = line.lstrip()
        if incomment:
            incomment = '*/' not in line
            out.append(line)
            continue
        if s.startswith(('/*', '//', '*')):
            incomment = s.startswith('/*') and '*/' not in line
            out.append(line)
            continue
        nxt = lines[i + 1] if i + 1 < len(lines) else ''
        new = line
        if '__stdargs' in line or not s:
            pass
        elif TYPEDEF.match(line):
            m = TYPEDEF.match(line)
            new = m.group(1) + '__stdargs ' + line[len(m.group(1)):]
        elif SKIP.match(line) or '=' in line.split('(')[0]:
            pass
        elif not line[0].isspace() and TYPEONLY.match(line) and NAMEPAREN.match(nxt):
            new = head(line)
        elif FNPTR.match(line):
            new = head(line)
        elif DECL.match(line) and (not line[0].isspace()
                                   or line.rstrip().endswith((';', ','))):
            new = params(head(line))
        out.append(new)
    return '\n'.join(out)


mode, path = sys.argv[1], sys.argv[2]
with open(path, encoding='utf-8', errors='surrogateescape') as f:
    text = f.read()
sys.stdout.write(strip(text) if mode == 'strip' else add(text))
