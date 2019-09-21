"""Generate a cmake module to create a target for a Verilog module.

"""

import os
import sys
import re
import argparse
import subprocess

import jinja2
import colorama


# Sentinel object to indicate if output should to to stdout.
CMAKE_STDOUT = object()


CMAKE_TEMPLATE = r'''
# ============================================================================
# "V{{ verilog_module_name }}" library target
#   Verilated {{ verilog_module_path }}
# ============================================================================

# This file was auto-generated!
# DO NOT EDIT!


add_library(V{{ verilog_module_name }}
    {% for filename in generated_source_files -%}
    "{{ filename }}"
    {% endfor -%}
    "{{ verilator_include_dir }}/verilated.cpp"
)

target_include_directories(V{{ verilog_module_name }}
    SYSTEM PUBLIC
        "{{ verilator_output_dir }}"
        "{{ verilator_include_dir }}"
)

add_custom_command(
    OUTPUT
        {% for filename in outputs -%}
        "{{ filename }}"
        {% endfor %}
    COMMAND
        python
        "{{ __file__ }}"
        "{{ verilog_module_path }}"
        {% for include_path in verilog_include_paths -%}
        "-I{{ include_path }}"
        {% endfor -%}
        --verilator-output-dir "{{ verilator_output_dir }}"
        --verilator-include-dir "{{ verilator_include_dir }}"
        --cmake "{{ cmake_output_path }}"

    DEPENDS
        {% for filename in depends -%}
        "{{ filename }}"
        {% endfor %}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running Verilator on \"{{ verilog_module_path }}\""
    VERBATIM
)

'''

DEFINES_HEADER_TEMPLATE = r'''
// ============================================================================
//   Verilated {{ verilog_module_path }} defines
// ============================================================================

// This file was auto-generated!
// DO NOT EDIT!

#ifndef V{{ verilog_module_name | upper }}_DEFINES_H
#define V{{ verilog_module_name | upper }}_DEFINES_H

{% for identifier, value in defines %}
#ifdef SVDEF_{{ identifier }}
#error SVDEF_{{ identifier }} is already defined, so the Verilog define cannot be used!
#else
#define SVDEF_{{ identifier }}  {{ value }}
#endif
{% endfor %}

#endif

'''


class Depfile(object):

    def __init__(self, outputs, depends):
        self.outputs = outputs
        self.depends = depends

    @classmethod
    def parse(cls, args, module_name):
        depfiles = [
            os.path.join(args.verilator_output_dir, filename)
            for filename in os.listdir(args.verilator_output_dir) if
            os.path.splitext(filename)[1] == '.d'
        ]
        if not depfiles:
            return cls(set(), set())
        else:
            assert len(depfiles) == 1

        outputs = set()
        depends = set()
        with open(depfiles[0], 'r') as f:
            for line in f:
                line_outputs, line_depends = line.split(':')
                outputs.update(line_outputs.split())
                depends.update(line_depends.split())

        if args.cmake_module_path is not CMAKE_STDOUT:
            outputs.add(args.cmake_module_path)

        # Remove the dependence on the Verilator binary as CMake
        # on MacOS thinks of it as a local file.
        # FUTURE: This should be a dependency, but an absolute one.
        depends.discard('verilator_bin')

        return cls(outputs, depends)

    def outputs_of_extension(self, extension):
        return [
            path for path in self.outputs
            if os.path.splitext(path)[1] == extension
        ]

    @property
    def oldest_output(self):
        timestamps = []
        for path in self.outputs:
            if os.path.exists(path):
                timestamps.append(os.path.getmtime(path))
        return min(timestamps)

    @property
    def newest_depend(self):
        timestamps = []
        for path in self.depends:
            if os.path.exists(path):
                timestamps.append(os.path.getmtime(path))
        return max(timestamps)

    @property
    def needs_regen(self):
        return self.newest_depend > self.oldest_output

    def __eq__(self, other):
        return (self.outputs, self.depends) == (other.outputs, other.depends)

    def __ne__(self, other):
        return not self.__eq__(other)


class VerilatorError(RuntimeError):

    def __init__(self, cmd, status, stdout, stderr):
        self.cmd = cmd
        self.status = status
        self.stdout = stdout
        self.stderr = stderr

    def print_stderr(self):
        for line in self.stderr.splitlines():
            if line.startswith('%Error'):
                print(f'{colorama.Fore.RED}{line}{colorama.Style.RESET_ALL}', file=sys.stderr)
            elif line.startswith('%Warning'):
                print(f'{colorama.Fore.YELLOW}{line}{colorama.Style.RESET_ALL}', file=sys.stderr)
            else:
                print(line, file=sys.stderr)


def run_verilator(verilator_args, output_dir):
    try:
        os.makedirs(output_dir)
    except FileExistsError:
        pass

    cmd = ['verilator_bin', *verilator_args, '--Mdir', output_dir]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()
    stdout = stdout.decode('utf-8')
    stderr = stderr.decode('utf-8')

    if p.returncode != 0:
        raise VerilatorError(cmd, p.returncode, stdout, stderr)
    return stdout


def verilate_source_files(args):
    output_dir = args.verilator_output_dir
    verilator_args = [
        '-Wall', '-cc', '--MMD',
        *[f'-I{path}' for path in args.verilog_include_paths],
        '-cc', args.verilog_module,
    ]
    print(run_verilator(verilator_args, output_dir))


def get_verilog_defines(args):
    output_dir = f'{args.verilator_output_dir}_defines'
    verilator_args = [
        '-E', '--dump-defines', '--lint-only',
        *[f'-I{path}' for path in args.verilog_include_paths],
        args.verilog_module,
    ]
    stdout = run_verilator(verilator_args, output_dir)

    raw_defines = []
    for line in stdout.splitlines():
        directive, identifier, *value = line.split(None, 2)
        value = value[0] if value else ''
        raw_defines.append((directive, identifier, value))

    defines = {}
    while raw_defines:
        directive, identifier, value = raw_defines.pop()

        if directive != '`define':
            continue

        # If a new define aliases a previous one, then use its value.
        # If we haven't processed it yet, then wait.
        if value and value[0] == '`':
            alias = value[1:]
            try:
                value = defines[alias]
            except KeyError:
                continue

        # Try to decode integer literals.
        if value is not None:
            pattern = r"\d+'([bdoh])(\d+)"
            match = re.match(pattern, value.lower())
            if match:
                base = {'b': 2, 'd': 10, 'o': 8, 'h': 16}[match.group(1)]
                value = f'{int(match.group(2), base)}     /* {match.group(0)} */'

        defines[identifier] = value
    return defines


def main():
    parser = argparse.ArgumentParser(description='Run Verilator and generate CMake module')
    parser.add_argument(
        'verilog_module',
        help='Path to top-level Verilog module',
    )
    parser.add_argument(
        '-I',
        dest='verilog_include_paths',
        action='append',
        help='Add a Verilog include path',
    )
    parser.add_argument(
        '--cmake',
        nargs='?',
        dest='cmake_module_path',
        const=CMAKE_STDOUT,
        help=('Generate CMake module at the given path. '
              'If no path is given, output is written to stdout.'),
    )
    parser.add_argument(
        '--verilator-output-dir',
        help='Verilator output directory',
    )
    parser.add_argument(
        '--verilator-include-dir',
        default='/usr/share/verilator/include',
        help='Location of Verilator installation include directory',
    )
    args = parser.parse_args()

    # FUTURE: Regex against the file instead?
    module_name = os.path.splitext(os.path.basename(args.verilog_module))[0]
    defines_header_path = os.path.join(args.verilator_output_dir, f'V{module_name}_defines.h')
    common_parameters = {
        'verilog_module_name': module_name,
        'verilog_module_path': args.verilog_module,
        'verilog_include_paths': args.verilog_include_paths,
        'verilator_output_dir': args.verilator_output_dir,
        'verilator_include_dir': args.verilator_include_dir,
        '__file__': __file__,
    }

    environment = jinja2.Environment(undefined=jinja2.StrictUndefined)

    def handle_verilator_error(exc):
        print(' '.join(exc.cmd))
        exc.print_stderr()
        print(f'ERROR: Verilator exited with status {exc.status}', file=sys.stderr)

    def get_depfile():
        depfile = Depfile.parse(args, module_name)
        depfile.outputs.add(defines_header_path)
        depfile.depends.add(__file__)
        return depfile

    try:
        previous_depfile = get_depfile()
    except FileNotFoundError:
        previous_depfile = None

    try:
        verilate_source_files(args)
    except VerilatorError as exc:
        handle_verilator_error(exc)
        return 1

    # TODO: Don't always regenerate this file
    try:
        verilog_defines = get_verilog_defines(args)
    except VerilatorError as exc:
        handle_verilator_error(exc)
        return 1
    template = environment.from_string(DEFINES_HEADER_TEMPLATE)
    content = template.render(
        **common_parameters,
        defines=sorted(verilog_defines.items()),
    )
    with open(os.path.join(args.verilator_output_dir, f'V{module_name}_defines.h'), 'w') as f:
        f.write(content)

    if args.cmake_module_path is not None:
        depfile = get_depfile()
        parameters = {
            **common_parameters,
            'outputs': sorted(depfile.outputs),
            'depends': sorted(depfile.depends),
            'generated_source_files': sorted(depfile.outputs_of_extension('.cpp')),
            'generated_header_files': sorted(depfile.outputs_of_extension('.h')),
            'cmake_output_path': '' if args.cmake_module_path is CMAKE_STDOUT else args.cmake_module_path,
        }
        template = environment.from_string(CMAKE_TEMPLATE)
        content = template.render(**parameters)
        if args.cmake_module_path is CMAKE_STDOUT:
            print(content)
        else:
            # TODO: Reset timestamps on cpp and h files which haven't
            # changed to prevent them being needlessly rebuilt?

            cmake_dirty_criteria = [
                not os.path.exists(args.cmake_module_path),
                depfile != previous_depfile,
                depfile.needs_regen,
            ]
            if any(cmake_dirty_criteria):
                # TODO: Is this not working?
                with open(args.cmake_module_path, 'w') as f:
                    f.write(content)
            else:
                print(f'-- {args.cmake_module_path} up-to-date.')


if __name__ == '__main__':
    sys.exit(main())
