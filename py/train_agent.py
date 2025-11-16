# py/train_agent.py
import torch, torch.nn as nn, torch.optim as optim
import random, json, argparse
from agent_server_ml import PolicyNet

def make_dataset(N=5000):
    X = []
    Y = []
    for _ in range(N):
        a = random.randint(0,255)
        b = random.randint(0,255)
        last = random.randint(0,255)
        # simple labeling rule used to create train data:
        if (a%2==1) or (b%2==1):
            y=4 # xor
        elif a < b:
            y=1 # sub
        else:
            y=0 # add
        X.append([a/255.0, b/255.0, last/255.0])
        Y.append(y)
    return torch.tensor(X,dtype=torch.float32), torch.tensor(Y,dtype=torch.long)

def train(save_path="policy.pt", epochs=10):
    X,Y = make_dataset(2000)
    model = PolicyNet()
    opt = optim.Adam(model.parameters(), lr=1e-3)
    crit = nn.CrossEntropyLoss()
    for ep in range(epochs):
        perm = torch.randperm(X.size(0))
        totloss=0.0
        for i in range(0, X.size(0), 64):
            idx = perm[i:i+64]
            xb = X[idx]; yb = Y[idx]
            logits = model(xb)
            loss = crit(logits, yb)
            opt.zero_grad(); loss.backward(); opt.step()
            totloss += loss.item()
        print(f"ep {ep} loss {totloss:.4f}")
    torch.save(model.state_dict(), save_path)
    print("Saved", save_path)

if __name__ == "__main__":
    import argparse
    p=argparse.ArgumentParser()
    p.add_argument("--out", default="policy.pt")
    p.add_argument("--epochs", type=int, default=10)
    args = p.parse_args()
    train(args.out, args.epochs)
