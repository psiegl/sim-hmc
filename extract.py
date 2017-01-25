import sqlite3
import pprint
conn = sqlite3.connect("hmcsim.db")
c = conn.cursor()

sql = """ 
SELECT tbl1.fromCubId AS            fromCubId,
       tbl1.toCubId AS              toCubId,
       tbl1.fromId AS               fromId,
       tbl1.toId AS                 toId,
       tbl1.linkTypeId AS           linkTypeId,
       (tbl2.cycle - tbl1.cycle) AS cycles,
    /* tbl1.physicalPktAddr AS      physicalPktAddr, */
       tbl1.pktTail AS              pktTail,
       tbl1.pktHeader AS            pktHeader
FROM
    (
        SELECT * FROM hmcsim_stat WHERE linkIntTypeId = 0 OR linkIntTypeId = 1
    ) AS tbl1
INNER JOIN
    (
        SELECT * FROM hmcsim_stat WHERE linkIntTypeId = 2 OR linkIntTypeId = 3
    ) AS tbl2
ON
   tbl1.physicalPktAddr = tbl2.physicalPktAddr AND
   tbl1.fromId = tbl2.fromId AND
   tbl1.toId = tbl2.toId AND
   tbl1.pktTail = tbl2.pktTail AND
   tbl1.pktHeader = tbl2.pktHeader AND
   (tbl1.linkIntTypeId+2) = tbl2.linkIntTypeId AND
   tbl1.cycle <= tbl2.cycle AND
   tbl1.toCubId = tbl2.toCubId AND
   tbl1.fromCubId = tbl2.fromCubId AND
   tbl1.linkTypeId = tbl2.linkTypeId
"""
 
#enum hmc_link_type {
#  HMC_LINK_EXTERN    = 0x0, /* HOST <-> DEV (SLID) or DEV <-> DEV */
#  HMC_LINK_RING      = 0x1,
#  HMC_LINK_VAULT     = 0x2
#};

type_sql = """
SELECT
  fromCubId,
  toCubId,
  fromId,
  toId,
  cycles,
  pktTail,
  pktHeader,
  CASE
    WHEN linkTypeId = 0 THEN 'link'
    WHEN linkTypeId = 1 THEN 'ring'
    WHEN linkTypeId = 2 THEN 'vault_in'
    WHEN linkTypeId = 3 THEN 'vault_out'
    WHEN linkTypeId = 4 THEN 'slid'
    ELSE 'undefined'
  END AS linkType
FROM (%s);
""" % sql


ctr = 0

cube_analysis = {}
for row in c.execute(type_sql):
    cube_analysis.setdefault(cube, {})

    graphviz = ''
    if row[7] == 'ring':
      cube = row[0]
      tag = "quad%d to quad%d" % (row[2],row[3])
      graphviz = "HMC%d:link%d -> HMC%d:link%d" % (cube, row[2], cube, row[3])
    elif row[7] == 'vault_in':
      cube = row[0]
      tag = "quad%d to vault%d" % (row[2],row[3])
    elif row[7] == 'vault_out':
      cube = row[0]
      tag = "vault%d to quad%d" % (row[2],row[3])
    elif row[7] == 'link':
      cube = row[0]
      tag = "hmc%d (quad%d) to hmc%d (quad%d)" % (row[0], row[2], row[1], row[3])
      graphviz = "HMC%d:link%d -> HMC%d:link%d" % (row[0], row[2], row[1], row[3])
    elif row[7] == 'slid' and row[0] == -1:
      cube = row[0]
      tag = "slid%d to hmc%d (quad%d)" % (row[2], row[1], row[3])
      graphviz = "HOST:slid%d -> HMC%d:link%d" % (row[2], row[1], row[3])
    else: # slid row[1] == -1
      cube = row[1]
      tag = "hmc%d (quad%d) to slid%d" % (row[0], row[2], row[3])
      graphviz = "HMC%d:link%d -> HOST:slid%d" % (row[0], row[2], row[3])

    cube_analysis[cube].setdefault( tag, { 'sum latency' : 0, "ctr pkt" : 0 } )
    cube_analysis[cube][tag]['sum latency'] += row[4]
    cube_analysis[cube][tag]['ctr pkt'] += 1
    if graphviz != '':
      cube_analysis[cube][tag]['graphviz'] = graphviz
    ctr += 1
#print( ctr )



graphviz = []
for cub, c_data in cube_analysis.items():
    for link, l_data in c_data.items():
        cube_analysis[cub][link]['avg latency'] = l_data['sum latency'] / l_data['ctr pkt']
        if 'graphviz' in l_data:
            cube_analysis[cub][link]['graphviz'] += " [label=\"%d clks avg. lat., %d pkts\"];" % (l_data['sum latency'] / l_data['ctr pkt'], l_data['ctr pkt'])
            graphviz.append( cube_analysis[cub][link]['graphviz'] )
print("""
digraph G {
    overlap = false;
    node[style=rounded, shape=record];
    HOST[label="{ HOST|{<slid0>slid0|<slid1>slid1|<slid2>slid2|<slid3>slid3} }"];
    HMC0[label="{ {<link0>link0||<link1>link1}|HMC0|{<link2>link2||<link3>link3} }"];
    HMC1[label="{ {<link0>link0||<link1>link1}|HMC1|{<link2>link2||<link3>link3} }"];

""")
print("}")

#pprint.pprint(cube_analysis)

conn.close()
