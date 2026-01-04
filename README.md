<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
</head>
<body>

<h1>Order Book Simulator (C++)</h1>

<p>
A price–time priority <strong>limit order book</strong> and matching engine written
in modern C++, with a high-volume load generator, correctness tests, and CSV export
for market data analysis.
</p>

<h2>Features</h2>
<ul>
  <li>Limit and market orders</li>
  <li>Cancel and modify support</li>
  <li>FIFO matching at each price level (price–time priority)</li>
  <li>Trade generation with integer timestamps</li>
  <li>High-volume randomized load testing (up to 100M operations)</li>
  <li>Invariant checks to ensure book correctness</li>
  <li>CSV export of L1 market data (best bid/ask, mid-price) for plotting</li>
</ul>

<h2>Design Overview</h2>
<ul>
  <li><strong>Price levels:</strong> <code>std::map&lt;price, level&gt;</code> for sorted access (O(log N))</li>
  <li><strong>Order queues:</strong> <code>std::list&lt;Order&gt;</code> per price level for FIFO execution (O(1))</li>
  <li><strong>Order handles:</strong> stored iterators enable O(1) cancellation</li>
  <li><strong>Matching:</strong> deterministic crossing logic with partial fills</li>
  <li><strong>Testing:</strong> assertion-based unit tests and randomized fuzz testing</li>
</ul>

<p>
The core matching engine is kept free of I/O; logging and analysis live in separate
applications.
</p>

<h2>Project Structure</h2>
<pre>
include/     Public headers (OrderBook API, shared types)
src/         Core matching engine implementation
apps/        CLI and benchmark executables
tests/       Correctness and invariant tests
scripts/     Analysis and plotting tools (Python)
docs/        Design notes and future improvements
</pre>

<h2>Build</h2>
<p>Requires a C++20 compiler and CMake ≥ 3.16.</p>

<pre>
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
</pre>

<h2>Run</h2>
<pre>
./build/orderbook_cli
./build/orderbook_bench
./build/orderbook_tests
</pre>

<h2>Performance</h2>
<p>
The benchmark driver generates randomized order flow and reports throughput.
</p>

<pre>
ops: 10,000,000
time: 1.8s
throughput: ~5.5M ops/sec
</pre>

<p><em>(Results are machine-dependent.)</em></p>

<h2>Testing</h2>
<p>Tests validate:</p>
<ul>
  <li>Non-crossed book invariant (<code>bestBid ≤ bestAsk</code>)</li>
  <li>FIFO execution at a single price level</li>
  <li>Correct handling of cancel and modify</li>
  <li>Stability under randomized workloads</li>
</ul>

<pre>
ctest --test-dir build
</pre>

<h2>Limitations</h2>
<ul>
  <li>Uses <code>std::map</code>, which has poor cache locality compared to production engines</li>
  <li>Single-threaded (no concurrency or locking)</li>
  <li>Simulated time; no real market data feed</li>
  <li>No persistence or networking layer</li>
</ul>

<h2>Future Work</h2>
<ul>
  <li>Replace <code>std::map</code> with array-based or flat-tree structures</li>
  <li>Add latency measurement (p50 / p95 / p99)</li>
  <li>Support L2 depth snapshots</li>
  <li>Deterministic replay from recorded event streams</li>
  <li>Multi-threaded matching and ingestion</li>
</ul>

<h2>Motivation</h2>
<p>
This project was built to understand the internal mechanics of electronic markets,
matching engines, and performance trade-offs in low-latency systems.
</p>

<hr>

<p>
<strong>Note:</strong> This project is intended as an educational and experimental
implementation, not a production exchange engine.
</p>

</body>
</html>
