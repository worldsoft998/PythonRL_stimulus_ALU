# py/agent_server_ml.py
import socket, struct, json, argparse, sys
import torch
import torch.nn as nn
import numpy as np

class PolicyNet(nn.Module):
    def __init__(self, in_dim=3, hidden=32, out_dim=5):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(in_dim, hidden),
            nn.ReLU(),
            nn.Linear(hidden, hidden),
            nn.ReLU(),
            nn.Linear(hidden, out_dim)
        )
    def forward(self,x): return self.net(x)

OP_MAP = {0:"add",1:"sub",2:"and",3:"or",4:"xor"}

def recv_msg(conn):
    raw = conn.recv(4)
    if not raw or len(raw) < 4: return None
    (length,) = struct.unpack(">I", raw)
    data = b''
    while len(data) < length:
        more = conn.recv(length - len(data))
        if not more: return None
        data += more
    return data.decode()

def send_msg(conn, msg):
    b = msg.encode()
    conn.sendall(struct.pack(">I", len(b)) + b)

def get_action_from_logits(logits, a,b,last):
    probs = torch.softmax(logits, dim=-1).detach().cpu().numpy().flatten()
    idx = int(np.argmax(probs))  # greedy for PoC; could sample
    return OP_MAP[idx]

def handle_conn(conn, model):
    while True:
        msg = recv_msg(conn)
        if msg is None:
            print("connection closed")
            break
        try:
            arr = json.loads(msg)
            if not isinstance(arr, list): arr = [arr]
        except Exception as e:
            print("bad json", e); continue
        actions = []
        # inference in batch
        X = []
        for obs in arr:
            a = obs.get("a",0); b = obs.get("b",0); last = obs.get("last_result",0)
            X.append([a/255.0, b/255.0, last/255.0])
        X = torch.tensor(X, dtype=torch.float32)
        if model is not None:
            with torch.no_grad():
                logits = model(X)
            for i, row in enumerate(arr):
                op = get_action_from_logits(logits[i], row.get("a",0), row.get("b",0), row.get("last_result",0))
                actions.append({"type":"action","seq_id": row.get("seq_id",0), "op":op, "a": row.get("a",0), "b": row.get("b",0)})
        else:
            # fallback heuristic
            for row in arr:
                a = row.get("a",0); b = row.get("b",0)
                if (a%2==1) or (b%2==1): op="xor"
                elif a < b: op="sub"
                else: op="add"
                actions.append({"type":"action","seq_id": row.get("seq_id",0), "op":op, "a":row.get("a",0), "b":row.get("b",0)})
        send_msg(conn, json.dumps(actions))

def main(port=5000, model_path=None):
    model = None
    if model_path:
        try:
            model = PolicyNet()
            model.load_state_dict(torch.load(model_path))
            model.eval()
            print("Loaded model", model_path)
        except Exception as e:
            print("Failed to load model:", e)
            model = None

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("0.0.0.0", port))
    s.listen(1)
    print("Policy server listening on", port)
    conn, addr = s.accept()
    print("Connected by", addr)
    try:
        handle_conn(conn, model)
    finally:
        conn.close()
        s.close()

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--port", type=int, default=5000)
    p.add_argument("--model", default=None)
    args = p.parse_args()
    main(args.port, args.model)
