# Chemical Synthesizer Graph

A graph-based chemical synthesis path finder with condition-based edge traversal.

## Features
- Graph-based synthesis path finding using Dijkstra's algorithm
- Condition-based edge activation (inventory system)
- Visual graph generation using Graphviz
- Cross-platform support (Windows, Linux, macOS)

## Build & Run

### Linux/macOS
```bash
./run.sh
```

### Windows
```cmd
run.bat
```

## Requirements
- GCC compiler
- Graphviz (for visualization)
  - Linux: `apt-get install graphviz`
  - macOS: `brew install graphviz`
  - Windows: Download from [graphviz.org](https://graphviz.org)

## Usage

### Commands
- `addNode <label>` - Add a node
- `removeNode <label>` - Remove a node
- `updateNode <old> <new>` - Rename a node
- `addEdge <from> <to> <weight>,<condition>` - Add edge with weight & optional condition
- `removeEdge <from> <to>` - Remove edges from→to
- `addCondition <cond>` - Add condition to inventory
- `removeCondition <cond>` - Remove condition from inventory
- `showInventory` - List all current conditions
- `show` - Display graph adjacency list
- `synthesize <start> <goal>` - Find optimal synthesis path
- `visualize <name>` - Export graph as PNG image
- `help` - Show help message
- `exit` - Exit program

### Condition Syntax
- Single condition: `glove`
- AND conditions: `glove & mask`
- OR conditions: `glove | mask`
- Complex: `(glove & mask) | high_temp`

## Example
```
addNode A
addNode B
addNode C
addEdge A B 5,
addEdge A C 10,glove
addCondition glove
synthesize A C
visualize graph1
```

## Improvements in Latest Version
- Fixed buffer overflow vulnerabilities
- Replaced unsafe `strtok` with thread-safe `strtok_r`
- Added cross-platform compatibility (Windows support)
- Enhanced error handling and input validation
- Automatic directory creation for images