import sqlite3
conn = sqlite3.connect("hmcsim.db")
c = conn.cursor()

for row in c.execute("SELECT * FROM hmcsim_stat;"):
    print( row )

conn.close()
