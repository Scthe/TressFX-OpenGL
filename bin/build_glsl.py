import re
import sys
import subprocess
from os import path

SHADERS_DIR = 'src/shaders'
SHADERS_OUT_DIR = 'src/shaders/generated'

SHADER_FILES = [
    'gl-tfx/ppll_build.vert.glsl',
    'gl-tfx/ppll_build.frag.glsl',
    'gl-tfx/ppll_resolve.vert.glsl',
    'gl-tfx/ppll_resolve.frag.glsl'
]

INCLUDE_REGEX = '^#pragma include "(.*?)"'
NVIDIA_ERR_REGEX = '^.*\((.*?)\).+?(e.*)$'

class Colors:
    BLACK   = '\033[0;{}m'.format(30)
    RED     = '\033[0;{}m'.format(31)
    GREEN   = '\033[0;{}m'.format(32)
    YELLOW  = '\033[0;{}m'.format(33)
    BLUE    = '\033[0;{}m'.format(34)
    MAGENTA = '\033[0;{}m'.format(35)
    CYAN    = '\033[0;{}m'.format(36)
    WHITE   = '\033[0;{}m'.format(37)

def process_file(filepath):
  buf = []
  with open(filepath) as f:
    lines = f.readlines()
    for line in lines:
      included_file = re.search(INCLUDE_REGEX, line, re.IGNORECASE)
      if included_file:
        file_to_include = path.join(path.dirname(filepath), included_file.group(1))
        print('  include: ', file_to_include)
        buf.extend(process_file(file_to_include))
      else:
        buf.append(line)
  return buf

def process_error_line(err_line, shader_lines):
    matches = re.search(NVIDIA_ERR_REGEX, err_line, re.IGNORECASE)
    if matches:
        line_no, msg = int(matches.group(1)), matches.group(2)
        line = shader_lines[line_no - 1].strip()
        return '[L{}] {}\n{}  > {}'.format(line_no, msg, Colors.CYAN, line)
    else:
        return err_line

def test_compile(filepath, shader_lines):
    ret = subprocess.check_output([
        'bin\glslcompiler\glslcompiler.exe',
        filepath
    ]).decode("utf-8")
    # print(Colors.RED, str(ret), Colors.WHITE)

    if 'success!' in ret.lower():
        return True

    err_lines = ret.split('\n')
    print(Colors.RED, err_lines[0], Colors.WHITE)
    for line in err_lines[1:]:
        print(Colors.RED, process_error_line(line, shader_lines), Colors.WHITE)
    return False


if __name__ == '__main__':
    has_error = False
    for filepath in SHADER_FILES:
        in_path = path.join(SHADERS_DIR, filepath)
        out_path = path.join(SHADERS_OUT_DIR, filepath)
        print(in_path, ' => ', out_path)

        after_processing = process_file(in_path)
        new_file_content = ''.join(after_processing)

        with open(out_path, 'w') as f:
            f.write(new_file_content)

        has_error = has_error or not test_compile(out_path, after_processing)
        # break

    print(Colors.YELLOW, "TODO Depth was mul *255", Colors.WHITE)
    if has_error:
        sys.exit(1)
    print()
