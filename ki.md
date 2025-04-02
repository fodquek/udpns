# 2025-April-02 | windows | closing with magic word causes error
- peer_demo A
```bash
# SENDER
RX UP
TX UP
KAPAT # my entry
send successful
TX KAPAT GELDI
recvform error
RECEIVE ERR
# finish
```
- peer_demo B
```bash
RX UP
TX UP
>>> KAPAT <<< # received my "peer_demo A" side entry
RX KAPAT GELDI
KAPAT # my entry
send error
TX KAPAT GELDI
# finish
```
