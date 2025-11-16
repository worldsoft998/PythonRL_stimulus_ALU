This repo demonstrates RL(Reinforcement Learning) modules in full Python code for  testbench in UVM written in  SystemVerilog of an small ALU of 16 bit aimed at Hardware Verification Optimization - stimulus generation optimization to minimize test time and to maximize coverage. For Python-HDL/HLS/HVL bridges/ interactions /interfaces/communications modules., DPI-C will be used. OpenAI Gymnasium is adopted as the RL agent. 

Proof of Concept: UVM transactor <-> Python Agent (batched) for ALU (VCS)
=========================================================

Contents
--------
- UVM testbench (minimal)
- DPI-C socket bridge with JSON parsing (embedded cJSON available)
- Batched protocol: send JSON array of observations, receive JSON array of actions

Requirements
------------
- Synopsys VCS (vcs in PATH)
- UVM (provided by your simulator)
- Python 3
- Optional: cJSON library (libcjson-dev) if you want to compile with system cJSON

Quick run (no system cJSON)
---------------------------
1. Start:
   ```
   make run
   ```
   This starts Python agent in background, compiles with VCS, runs simulation, then kills agent.

2. Or run manually:
   - Start agent: `python3 py/agent_server_ml.py`
   - In another terminal: `make sim`

With system cJSON support
-------------------------
Install cJSON (Ubuntu example): `sudo apt-get install libcjson-dev`
Then run:
```
make run USE_CJSON=1
```

Notes
-----
- The DPI-C source has two parsing modes: if compiled with `-DUSE_CJSON` it uses the system cJSON for parsing. Otherwise the embedded minimal cJSON.c/h is used.
- Batch payloads are JSON arrays; both sides use length-prefixed messages (4-byte big-endian length + payload).
- For production: use secure/authenticated channels, proper JSON libs, and robust error handling.
