## Cquel lite

A database with the wrong pronunciation of SQL


## Database Design

### Database Disk Representation

A database is represented by a directory and each table is represented by a file. This is middle ground from what postgres and sqlite does,
the latter uses a single file for the whole DB and the former uses 1 file per page (plus free space and visibility files)

### Table file

Each Table is represented by a collection of 4KB block/pages


In ASCII it would like like the following:

```
Table structure
 ----------------------------------------------------------
|               |         |         |           |         |
|  Table Header | Page 1  |  Page 2 |  ....     | Page N  |
|               |         |         |           |         |
 ----------------------------------------------------------

Page structure
+----------------|---------------------------------+
| PageHeader | linp1 linp2 linp3 ...               |
+-----------|----|---------------------------------+
| ... linpN |                                      |
+-----------|--------------------------------------+
|           ^ lower limit                          |
|                                                  |
|             v upper limit                        |
+-------------|------------------------------------+
|             | tupleN ...                         |
+-------------|------------------|-----------------+
|     ... tuple3 tuple2 tuple1                     |
+--------------------------------|-----------------+
```

### Buffer Pool

Since atm the only thing cquel-lite can do is sequential scans on single tables, the replacement policy is just a modulo of the page_id.
I might do a clock sweep LRU later in the process.


### Where is this project atm

[x] Table structure
[x] Page structure
[x] Tuple insertion
[x] Select all from a table
[ ] Cache/Buffer Pool (WIP)
[ ] Btree indexes
[ ] Dynamic row/column structure
[ ] Multiple Tables
[ ] Concurrent queries and tree locking
[ ] Moar Queries
[ ] Multiple DBs
[ ] Proper SQL parsing
[ ] Relational modelling
[ ] Transactions
