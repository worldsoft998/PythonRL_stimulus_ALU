# Makefile - VCS run for UVM batched RL algorithm for ALU verification optimization

VCS = vcs
VCS_FLAGS = -full64 -sverilog +vpi -debug_all
SIMV = ./simv

PY = python3
PY_AGENT = py/agent_server_ml.py

SRC_SV = sv/tb_top.sv sv/test.sv sv/env.sv sv/transactor.sv sv/driver.sv sv/seq_item.sv sv/seq.sv rtl/alu_dut.sv rtl/alu_if.sv
DPI_C = dpi/dpi_socket.c dpi/cJSON.c
INC =

# optional cJSON support: set USE_CJSON=1 when invoking make to compile with cJSON.
# e.g.: make run USE_CJSON=1
ifeq ($(USE_CJSON),1)
  CFLAGS += -DUSE_CJSON
  LDFLAGS += -lcjson
endif

.PHONY: all agent sim run clean

all: sim

agent:
	$(PY) $(PY_AGENT)

sim:
	@echo "Compiling with VCS..."
	$(VCS) $(VCS_FLAGS) $(SRC_SV) $(DPI_C) $(INC) $(CFLAGS) $(LDFLAGS) -o simv || (echo "VCS compile failed"; exit 1)
	@echo "Run simv..."
	$(SIMV)

run:
	@echo "Starting python agent in background..."
	$(PY) $(PY_AGENT) & echo $$! > agent.pid
	@sleep 0.2
	$(MAKE) sim
	@echo "Killing python agent..."
	@if [ -f agent.pid ]; then kill `cat agent.pid` || true; rm -f agent.pid; fi

clean:
	rm -rf simv csrc ucli.key DVEfiles *.log simv.daidir
