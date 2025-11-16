# py/collect_worker.py
# Example worker that queries the policy server repeatedly with random obs.
# This demonstrates a parallel data collection pattern (not launching VCS).
import socket, struct, json, random, time, argparse

def send_and_recv(host="127.0.0.1", port=5000, batch=8):
    with socket.create_connection((host,port)) as conn:
        def send_msg(msg):
            b=msg.encode(); conn.sendall(struct.pack(">I", len(b))+b)
        def recv_msg():
            raw=conn.recv(4)
            if not raw: return None
            (length,) = struct.unpack(">I", raw)
            data=b''
            while len(data)<length:
                m=conn.recv(length-len(data))
                if not m: return None
                data+=m
            return data.decode()
        obs=[]
        for i in range(batch):
            a=random.randint(0,255); b=random.randint(0,255)
            obs.append({"type":"obs","seq_id":i,"a":a,"b":b,"last_result":0})
        send_msg(json.dumps(obs))
        resp = recv_msg()
        print("Got", resp)

if __name__=="__main__":
    send_and_recv()
