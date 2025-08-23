import os
import re
import sys

# path to SDK dir
SDK_PATH = "./sdk"

# file extensions to search ( Leave default )
EXTENSIONS = [".cpp", ".h", ".hpp"]

# number of lines to include around the mathch
CONTEXT_LINES = 5

def search(query, max_results=10):
    results = []

    for root, dirs, files in os.walk(SDK_PATH):
        for file in files:
            if any(file.endswith(ext) for ext in EXTENSIONS):
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, "r", errors="ignore") as f:
                        lines = f.readlines()
                        for i, line in enumerate(lines):
                            if re.search(query, line, re.IGNORECASE):
                                start = max(i - CONTEXT_LINES, 0)
                                end = min(i + CONTEXT_LINES + 1, len(lines))
                                snippet = "".join(lines[start:end])
                                results.append({
                                    "file": filepath,
                                    "line": i + 1,
                                    "snippet": snippet
                                })
                except Exception as e:
                    print(f"[Failed] could not read {filepath}: {e}")
    
    # Return top N results
    return results[:max_results]

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("[Usage] python search.py <Query>")
        sys.exit(1)

    query = sys.argv[1]
    max_results = int(sys.argv[2]) if len(sys.argv) > 2 else 10

    matches = search(query, max_results=max_results)

    if not matches:
        print("[Failed] No matches found.")
    else:
        for match in matches:
            print(f"\File: {match['file']} (Line {match['line']})")
            print("-" * 60)
            print(match['snippet'])
            print("-" * 60)
