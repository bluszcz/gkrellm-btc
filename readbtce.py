#!/usr/bin/env python

import sys, httplib, json
cur = sys.argv[1:]
conn = httplib.HTTPSConnection("btc-e.com")
results = []
try:
    for i in cur:
        conn.request("GET", "/api/2/"+i+"_btc/ticker")
        r1 = conn.getresponse()
        data = r1.read()
        results.append(str(json.loads(data)['ticker']['sell']).strip())
    print ','.join(results)
except:
    print '0.0,0.0'
