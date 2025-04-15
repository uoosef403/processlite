#!/usr/bin/env python3

import os
import argparse
import re
import sys

# --- Configuration ---
# File extensions to consider
HEADER_EXTENSIONS = ('.h', '.hpp')
SOURCE_EXTENSIONS = ('.cpp', '.cxx', '.cc')
# Default directory exclude prefixes
DEFAULT_EXCLUDE_PREFIXES = ["cmake-", ".", "_"] # Exclude cmake-, hidden, and underscore folders by default

# --- Core Functions ---

def find_files(root_dir, exclude_prefixes):
    """
    Recursively finds header and source files in a directory tree,
    excluding specified directories.

    Args:
        root_dir (str): The root directory to start searching from.
        exclude_prefixes (list): A list of string prefixes. Directories
                                 starting with any of these prefixes will be excluded.

    Returns:
        tuple: A tuple containing two lists:
               - header_files (list): List of full paths to header files.
               - source_files (list): List of full paths to source files.
    """
    header_files = []
    source_files = []
    abs_root = os.path.abspath(root_dir)
    print(f"Starting search in: {abs_root}", file=sys.stderr)
    print(f"Excluding directories starting with: {exclude_prefixes}", file=sys.stderr)

    # Walk through the directory tree
    for dirpath, dirnames, filenames in os.walk(abs_root, topdown=True):
        # --- Directory Exclusion ---
        # Modify dirnames in-place to prevent os.walk from descending into excluded directories
        # Iterate backwards to safely remove items while iterating
        for i in range(len(dirnames) - 1, -1, -1):
            d = dirnames[i]
            # Check if the directory name starts with any of the exclude prefixes
            if any(d.startswith(prefix) for prefix in exclude_prefixes):
                print(f"  Excluding directory: {os.path.join(dirpath, d)}", file=sys.stderr)
                # Remove the directory from the list
                del dirnames[i]

        # --- File Collection ---
        for filename in filenames:
            # Construct the full path
            full_path = os.path.join(dirpath, filename)
            # Get the file extension
            _, ext = os.path.splitext(filename)
            ext = ext.lower()

            # Check for header files
            if ext in HEADER_EXTENSIONS:
                header_files.append(full_path)
            # Check for source files
            elif ext in SOURCE_EXTENSIONS:
                source_files.append(full_path)

    # Sort files for consistent bundling order
    header_files.sort()
    source_files.sort()

    print(f"Found {len(header_files)} header files and {len(source_files)} source files.", file=sys.stderr)
    return header_files, source_files

def bundle_files(header_files, source_files, output_filename):
    """
    Bundles the content of header and source files into a single output file.
    **This version KEEPS system includes (#include <...>) in place,
    but REMOVES local includes (#include "...")**

    Args:
        header_files (list): List of paths to header files.
        source_files (list): List of paths to source files.
        output_filename (str): The path to the output bundled file.
    """
    # Store content of each file (processed to handle includes)
    header_contents = {}
    source_contents = {}

    # Regex to find include directives
    # Handles #include <...>, #include "...", and potential whitespace variations
    include_regex = re.compile(r'^\s*#\s*include\s+([<"])([^>"]+)([>"])')

    print(f"Processing {len(header_files)} header files (removing local includes)...", file=sys.stderr)
    # --- First pass: Process Headers (Keep <...>, Remove "...") ---
    for file_path in header_files:
        content_lines = []
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as infile:
                # Read line by line
                for line_num, line in enumerate(infile, 1):
                    match = include_regex.match(line)
                    if match:
                        include_type = match.group(1)
                        # If it's a local include ("..."), skip it
                        if include_type == '"':
                            continue # Ignore the local include line
                        # Otherwise (it's a system include <...>), keep it
                        else:
                            content_lines.append(line)
                    else:
                        # Add non-include lines to the content
                        content_lines.append(line)
            # Store the processed content
            header_contents[file_path] = "".join(content_lines)
        except Exception as e:
            print(f"Warning: Could not read file {file_path}: {e}", file=sys.stderr)

    print(f"Processing {len(source_files)} source files (removing local includes)...", file=sys.stderr)
    # --- First pass: Process Sources (Keep <...>, Remove "...") ---
    for file_path in source_files:
        content_lines = []
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as infile:
                # Read line by line
                for line_num, line in enumerate(infile, 1):
                    match = include_regex.match(line)
                    if match:
                        include_type = match.group(1)
                        # If it's a local include ("..."), skip it
                        if include_type == '"':
                            continue # Ignore the local include line
                        # Otherwise (it's a system include <...>), keep it
                        else:
                            content_lines.append(line)
                    else:
                        # Add non-include lines to the content
                        content_lines.append(line)
            # Store the processed content
            source_contents[file_path] = "".join(content_lines)
        except Exception as e:
            print(f"Warning: Could not read file {file_path}: {e}", file=sys.stderr)


    # --- Second pass: Write to output file ---
    print(f"Writing bundled code (keeping system includes) to {output_filename}...", file=sys.stderr)
    try:
        with open(output_filename, 'w', encoding='utf-8') as outfile:
            # --- File Header ---
            outfile.write(f"// ==================================================\n")
            outfile.write(f"// Bundled C++ Code (Local #includes removed)\n")
            outfile.write(f"// System includes (#include <...>) are kept in place.\n")
            outfile.write(f"// Generated by script from:\n")
            if header_files:
                outfile.write(f"//   {len(header_files)} Header Files (.h, .hpp)\n")
            if source_files:
                outfile.write(f"//   {len(source_files)} Source Files (.cpp, .cxx, .cc)\n")
            outfile.write(f"// Target file: {os.path.basename(output_filename)}\n")
            outfile.write(f"// ==================================================\n\n")

            # --- Header Files Content ---
            outfile.write(f"// ==================================================\n")
            outfile.write(f"// Header Files Content ({len(header_files)} files)\n")
            outfile.write(f"// ==================================================\n\n")
            if not header_files:
                outfile.write("// --- No header files found ---\n\n")
            else:
                for file_path in header_files: # Use the sorted list
                    if file_path in header_contents:
                        rel_path = os.path.relpath(file_path)
                        outfile.write(f"// --- Content From: {rel_path} ---\n\n")
                        outfile.write(header_contents[file_path].strip() + "\n") # Add newline after content
                        outfile.write(f"\n// --- End Content From: {rel_path} ---\n\n")
                    else:
                        outfile.write(f"// --- Skipped (Error Reading?): {os.path.relpath(file_path)} ---\n\n")


            # --- Source Files Content ---
            outfile.write(f"// ==================================================\n")
            outfile.write(f"// Source Files Content ({len(source_files)} files)\n")
            outfile.write(f"// ==================================================\n\n")
            if not source_files:
                outfile.write("// --- No source files found ---\n\n")
            else:
                for file_path in source_files: # Use the sorted list
                    if file_path in source_contents:
                        rel_path = os.path.relpath(file_path)
                        outfile.write(f"// --- Content From: {rel_path} ---\n\n")
                        outfile.write(source_contents[file_path].strip() + "\n") # Add newline after content
                        outfile.write(f"\n// --- End Content From: {rel_path} ---\n\n")
                    else:
                        outfile.write(f"// --- Skipped (Error Reading?): {os.path.relpath(file_path)} ---\n\n")

            outfile.write(f"// ==================================================\n")
            outfile.write(f"// End of Bundled Code\n")
            outfile.write(f"// ==================================================\n")

        print(f"\nSuccessfully bundled files (keeping system includes) into {output_filename}", file=sys.stderr)

    except Exception as e:
        print(f"\nError writing to output file {output_filename}: {e}", file=sys.stderr)
        sys.exit(1) # Exit with error code

# --- Main Execution ---
if __name__ == "__main__":
    # Set up argument parser
    parser = argparse.ArgumentParser(
        description="Bundle C++ header and source files into a single file, keeping system includes (#include <...>) and removing local includes (#include \"...\").",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""\
Example Usage:
  python bundle_cpp.py ./my_project -o bundled_sys_includes.cpp -e build .git venv

This command will:
- Search for C++/H files in './my_project'.
- Exclude directories starting with 'build', '.git', or 'venv'.
- Output the bundled code to 'bundled_sys_includes.cpp'.
  (System includes are kept, local includes are removed).
"""
    )
    # Argument for the root directory to search
    parser.add_argument(
        "root_dir",
        nargs='?', # Optional argument
        default='.', # Default to current directory if not provided
        help="The root directory to search for C++/H files (default: current directory)."
    )
    # Argument for the output file name
    parser.add_argument(
        "-o", "--output",
        default="bundled_code_sys_includes.cpp",
        help="The name of the output bundled C++ file (default: bundled_code_sys_includes.cpp)."
    )
    # Argument for exclude prefixes
    parser.add_argument(
        "-e", "--exclude",
        metavar='PREFIX',
        nargs='*', # 0 or more arguments
        default=DEFAULT_EXCLUDE_PREFIXES, # Default exclude prefixes
        help=f"Prefix(es) of directories to exclude (default: {' '.join(DEFAULT_EXCLUDE_PREFIXES)})."
    )

    # Parse command-line arguments
    args = parser.parse_args()

    # Ensure exclude prefixes are strings
    exclude_prefixes = [str(p) for p in args.exclude]

    # Find header and source files, excluding specified directories
    header_files, source_files = find_files(args.root_dir, exclude_prefixes)

    # Check if any files were found
    if not header_files and not source_files:
        print("\nWarning: No header or source files found matching the criteria.", file=sys.stderr)
        try:
            with open(args.output, 'w', encoding='utf-8') as outfile:
                outfile.write(f"// Bundled C++ Code - Generated by script\n")
                outfile.write(f"// Target file: {os.path.basename(args.output)}\n")
                outfile.write(f"// No source or header files found in '{args.root_dir}' (excluding prefixes: {exclude_prefixes})\n")
                outfile.write(f"// NOTE: This script version removes local #includes and keeps system #includes.\n")

            print(f"Created empty output file: {args.output}", file=sys.stderr)
        except Exception as e:
            print(f"\nError writing empty output file {args.output}: {e}", file=sys.stderr)
            sys.exit(1)
        sys.exit(0) # Exit successfully, even if no files found


    # Bundle the found files into the output file
    bundle_files(header_files, source_files, args.output)
