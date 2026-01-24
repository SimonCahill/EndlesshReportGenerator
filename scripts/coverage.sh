#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"

echo "Running tests..."
ctest --test-dir "$BUILD_DIR" --output-on-failure

if command -v lcov >/dev/null 2>&1 && command -v genhtml >/dev/null 2>&1; then
  echo "Collecting coverage with lcov..."
  lcov --directory "$BUILD_DIR" --capture --output-file "$BUILD_DIR/coverage.info"
  lcov --remove "$BUILD_DIR/coverage.info" '/usr/*' --output-file "$BUILD_DIR/coverage.info"
  genhtml "$BUILD_DIR/coverage.info" --output-directory "$BUILD_DIR/coverage-report"
  echo "Coverage report generated at $BUILD_DIR/coverage-report/index.html"
  lcov --summary "$BUILD_DIR/coverage.info"
  exit 0
fi

echo "lcov/genhtml not found. Attempting gcov fallback..."
if ! command -v gcov >/dev/null 2>&1; then
  cat <<'EOF' >&2
gcov not found. To generate coverage you must run this on Linux with gcc/clang and
install lcov/genhtml or gcov.

Suggested steps on a Linux machine:
  sudo apt-get install lcov
  cmake -S . -B build -DENABLE_COVERAGE=ON
  cmake --build build
  ./scripts/coverage.sh
EOF
  exit 2
fi

echo "Running gcov on object files..."
pushd "$BUILD_DIR" >/dev/null
find . -name '*.gcda' -print0 | xargs -0 -n1 dirname | sort -u | while read -r dir; do
  echo "Processing $dir"
  # run gcov for all source files in the directory
  (cd "$dir" && for f in *.o ../src/*.cpp ../src/*.c 2>/dev/null; do true; done)
done
echo "gcov fallback cannot produce a nice HTML report here. Install lcov for best results." >&2
popd >/dev/null

exit 1
