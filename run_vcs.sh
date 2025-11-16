#!/bin/bash
set -e
python3 py/agent_server_ml.py & echo $! > agent.pid
sleep 0.2
make sim
if [ -f agent.pid ]; then kill $(cat agent.pid) || true; rm -f agent.pid; fi
