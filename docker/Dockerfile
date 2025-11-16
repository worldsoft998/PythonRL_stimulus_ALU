FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends     build-essential python3 python3-pip python3-dev git curl wget ca-certificates     pkg-config     libssl-dev     && rm -rf /var/lib/apt/lists/*

# Install pip packages including PyTorch CPU (change if you need GPU)
RUN pip3 install --no-cache-dir torch torchvision

# Copy repo into container
WORKDIR /workspace
COPY . /workspace

# Build the DPI C with embedded cJSON
RUN gcc -fPIC -shared dpi/dpi_socket.c dpi/cJSON.c -o /workspace/dpi/libdpi_socket.so

# Expose port for policy server
EXPOSE 5000

# default command: start the ML agent server (no model)
CMD [\"python3\", \"py/agent_server_ml.py\", \"--port\", \"5000\"]
