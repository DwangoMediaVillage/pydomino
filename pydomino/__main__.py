import os
import sys


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    if os.name == "nt":
        binary_path = os.path.join(script_dir, "domino.exe")
    else:
        binary_path = os.path.join(script_dir, "domino")

    if not os.path.exists(binary_path):
        print(f"Error: {binary_path} not found")
        sys.exit(1)

    os.execvp(binary_path, [binary_path] + sys.argv[1:])
