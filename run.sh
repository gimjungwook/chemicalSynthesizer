set -e
echo "🔨 Compiling..."
gcc synthesizer_graph.c -o synthesizer_graph

echo "🚀 Running test case..."
./synthesizer_graph < test_case.txt