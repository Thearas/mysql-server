--source include/elide_costs.inc

--echo #
--echo # WL#10358 Implement table value constructors: VALUES.
--echo #

--echo # Allowed: <table value constructor> with VALUES for a <simple table>
--echo # clause.

VALUES ROW(1, 10);
VALUES ROW(1, 10), ROW(2, 20);

--echo # Allowed: Table value constructor inside a subquery.

SELECT * FROM (VALUES ROW(1, 10)) AS dt;
--sorted_result
SELECT * FROM (VALUES ROW(1, 10), ROW(2, 20)) AS dt;

--echo # Allowed: Both old and new syntax for INSERT statements.

CREATE TABLE t0(a INT, b INT);

INSERT INTO t0 VALUES(1, 10);
--sorted_result
INSERT INTO t0 VALUES ROW(2, 20), ROW(3, 30);

SELECT * FROM t0;

DELETE FROM t0;

--echo # Not allowed: Row value expressions of different degree.

--error ER_WRONG_VALUE_COUNT_ON_ROW
VALUES ROW(1, 10), ROW(2);

--error ER_WRONG_VALUE_COUNT_ON_ROW
VALUES ROW(1), ROW(2, 20);

--echo # Allowed: Explicit table clause. Equivalent to SELECT * FROM table_name.

INSERT INTO t0 VALUES(1, 10);

TABLE t0;
SELECT * FROM (TABLE t0) AS dt;

DELETE FROM t0;

--echo # Allowed: Using table value constructors with JOIN.

INSERT INTO t0 VALUES(1, 10);

SELECT * FROM t0 JOIN (VALUES ROW(2, 20)) AS dt;
--sorted_result
SELECT * FROM t0 JOIN (VALUES ROW(2, 20), ROW(3, 30)) AS dt;

--sorted_result
SELECT * FROM t0
LEFT JOIN (VALUES ROW(2, 20), ROW(3, 30)) AS dt
ON t0.a = dt.column_0;

--sorted_result
SELECT * FROM t0
LEFT JOIN (VALUES ROW(1, 20), ROW(1, 30)) AS dt
ON t0.a = dt.column_0;

--sorted_result
SELECT * FROM t0
RIGHT JOIN (VALUES ROW(2, 20), ROW(3, 30)) AS dt
ON t0.a = dt.column_0;

--sorted_result
SELECT * FROM t0
RIGHT JOIN (VALUES ROW(1, 20), ROW(1, 30)) AS dt
ON t0.a = dt.column_0;

--sorted_result
SELECT * FROM (VALUES ROW(1), ROW(2)) AS dt0
NATURAL JOIN (VALUES ROW(1, 10), ROW(2, 20)) AS dt1;

--sorted_result
SELECT * FROM (VALUES ROW(1), ROW(2)) AS dt0
NATURAL JOIN (VALUES ROW(1, 10), ROW(1, 20)) AS dt1;

DELETE FROM t0;

--echo # Allowed: Using table value constructors with UNION.

VALUES ROW(1) UNION SELECT 2;
VALUES ROW(1, 10) UNION SELECT 2, 20;

--echo # Allowed: Scalar subquery as table value constructor value.

VALUES ROW((SELECT 1), 10);

--echo # Allowed: Aggregation of types in table values constructor rows.

VALUES ROW(1, 10), ROW(2, "20");

--echo # Allowed: Creating tables from aggregated types.

CREATE TABLE t AS VALUES ROW(1, 1.0, 1.0e0, DATE '2000-01-01', TIME '00:00:00',
                             "1", POINT(1, 1), JSON_ARRAY('[1, "2"]'));
DESC t;
DROP TABLE t;

CREATE TABLE t AS VALUES ROW(1, 1, 1, 1, 1, 1, 1, 1),
                         ROW(1, 1.0, 1.0e0, DATE '2000-01-01', TIME '00:00:00',
                             "1", POINT(1, 1), JSON_ARRAY('[1, "2"]'));
DESC t;
DROP TABLE t;

--echo # Allowed: Aggregation of types in table value constructor used in UNION.

--sorted_result
SELECT * FROM (VALUES ROW(1, 10), ROW(2, "20") UNION SELECT 3, 30) AS dt;

--echo # Note: Here the columns are named from the first SELECT instead.
--sorted_result
SELECT * FROM (SELECT 1, 10 UNION VALUES ROW(2, 20), ROW(3, "30")) AS dt;

--echo # Allowed: Reference an inner table in subqueries within rows.

INSERT INTO t0 VALUES(1, 10);

VALUES ROW((SELECT a FROM t0), 10);

DELETE FROM t0;

--echo # Allowed: Use table value constructor for subquery predicates.

INSERT INTO t0 VALUES(1, 10), (2, 20);

--sorted_result
SELECT * FROM t0 WHERE a IN (VALUES ROW(1));
--sorted_result
SELECT * FROM t0 WHERE a IN (VALUES ROW(1), ROW(2));
--sorted_result
SELECT * FROM t0 WHERE (a, b) IN (VALUES ROW(1, 10));
--sorted_result
SELECT * FROM t0 WHERE (a, b) IN (VALUES ROW(1, 10), ROW(2, 20));

--sorted_result
SELECT * FROM t0 WHERE a NOT IN (VALUES ROW(1));
--sorted_result
SELECT * FROM t0 WHERE a NOT IN (VALUES ROW(1), ROW(2));
--sorted_result
SELECT * FROM t0 WHERE (a, b) NOT IN (VALUES ROW(1, 10));
--sorted_result
SELECT * FROM t0 WHERE (a, b) NOT IN (VALUES ROW(1, 10), ROW(2, 20));

--sorted_result
SELECT * FROM t0 WHERE a >ALL (VALUES ROW(1));
--sorted_result
SELECT * FROM t0 WHERE a >ALL (VALUES ROW(1), ROW(2));

--sorted_result
SELECT * FROM t0 WHERE a <ANY (VALUES ROW(1));
--sorted_result
SELECT * FROM t0 WHERE a <ANY (VALUES ROW(1), ROW(2));

DELETE FROM t0;

--echo # Allowed: Table value constructor with empty rows in INSERT.

CREATE TABLE t1(a INT DEFAULT 1, b INT DEFAULT 10);

INSERT INTO t1 VALUES ROW();

SELECT * FROM t1;

DROP TABLE t1;

--echo # Not allowed: Table value constructor with empty rows outside INSERT.

--error ER_TABLE_VALUE_CONSTRUCTOR_MUST_HAVE_COLUMNS
VALUES ROW();

--echo # Allowed: Using non-deterministic functions.

VALUES ROW(RAND(0), RAND(1));

--echo # Allowed: Outer references.

INSERT INTO t0 VALUES(1, 10), (2, 20);

SELECT * FROM t0 WHERE b IN (VALUES ROW(a*10));

DELETE FROM t0;

--echo # Allowed: NULL in table value constructor.

VALUES ROW(1, NULL);
VALUES ROW(1, 10), ROW(2, NULL);

INSERT INTO t0 VALUES(1, 10);
SELECT * FROM t0 WHERE (a, b) IN (VALUES ROW(1, NULL));

INSERT INTO t0 VALUES ROW(1, NULL);
SELECT * FROM t0;

CREATE TABLE t AS VALUES ROW(1, NULL);
DESC t;

DELETE FROM t0;
DROP TABLE t;

--echo # Allowed: Non-deterministic functions with CTE.

--sorted_result
WITH v AS (VALUES ROW(RAND(0)), ROW(RAND(1)))
SELECT * FROM v;

--sorted_result
WITH v AS (VALUES ROW(RAND(0)), ROW(RAND(1)))
SELECT * FROM v AS v1 JOIN v AS v2;

--echo # Allowed: INSERT .. ON DUPLICATE KEY UPDATE with table value
--echo # constructor.

CREATE TABLE t(a INT PRIMARY KEY, b INT); INSERT INTO t VALUES(1, 10);

INSERT INTO t SELECT * FROM (VALUES ROW(1, 11), ROW(2, 20)) AS n(a, b)
ON DUPLICATE KEY UPDATE b= n.b;

SELECT * FROM t;

DROP TABLE t;

--echo # Not allowed: Table value constructor with DEFAULT if not part of INSERT
--echo # statement.

--error ER_TABLE_VALUE_CONSTRUCTOR_CANNOT_HAVE_DEFAULT
VALUES ROW(DEFAULT);

--error ER_TABLE_VALUE_CONSTRUCTOR_CANNOT_HAVE_DEFAULT
SELECT * FROM (VALUES ROW(DEFAULT)) AS dt;

CREATE TABLE t(a INT DEFAULT 1, b INT);

INSERT INTO t VALUES ROW(DEFAULT, 10);
INSERT INTO t VALUES ROW(DEFAULT, DEFAULT);
INSERT INTO t VALUES ROW(DEFAULT(a), 20);
INSERT INTO t VALUES ROW(DEFAULT(a) + 1, 30);

SELECT * FROM t;

DROP TABLE t;

DROP TABLE t0;

--echo Coverage for multiple data types (more than one row required)

VALUES ROW(1, 1.0, 1.0E0, '1', DATE'2000-01-01', TIME'00:00:01',
           TIMESTAMP'2000-01-01 00:00:01', CAST('{"j":"1"}' AS JSON)),
       ROW(2, 2.0, 2.0E0, '2', DATE'2000-01-02', TIME'00:00:02',
           TIMESTAMP'2000-01-01 00:00:01', CAST('{"j":"2"}' AS JSON));

--echo # View tests

--error ER_PARSE_ERROR
CREATE VIEW v AS VALUES;

CREATE VIEW v AS VALUES ROW(1);
SELECT * FROM v;
SELECT column_0 FROM v;
--error ER_BAD_FIELD_ERROR
SELECT column_x FROM v;
DROP VIEW v;

CREATE VIEW v(x) AS VALUES ROW(1);
SELECT * FROM v;
SELECT x FROM v;
--error ER_BAD_FIELD_ERROR
SELECT column_x FROM v;
DROP VIEW v;

CREATE VIEW v AS SELECT * FROM (VALUES ROW(1)) AS t1;
SELECT * FROM v;
SELECT column_0 FROM v;
--error ER_BAD_FIELD_ERROR
SELECT column_x FROM v;
DROP VIEW v;

CREATE VIEW v AS SELECT * FROM (VALUES ROW(1)) AS t1(x);
SELECT * FROM v;
SELECT x FROM v;
--error ER_BAD_FIELD_ERROR
SELECT column_x FROM v;
DROP VIEW v;

CREATE VIEW v(x) AS SELECT * FROM (VALUES ROW(1)) AS t1(z);
SELECT * FROM v;
SELECT x FROM v;
--error ER_BAD_FIELD_ERROR
SELECT column_x FROM v;
DROP VIEW v;

CREATE VIEW v(x, y) AS VALUES ROW(1,2), ROW(2,4), ROW(3,6);
SELECT * FROM v;
SELECT x, y FROM v;
SELECT MIN(x), MAX(y), SUM(x), SUM(y) FROM v;

CREATE TABLE t(a INTEGER, b INTEGER);
INSERT INTO t VALUES(1, 10), (2, 20);
SELECT * FROM v JOIN t ON v.x=t.a;
SELECT * FROM v LEFT JOIN t ON v.x=t.a;
DROP TABLE t;
DROP VIEW v;

--echo #
--echo # Bug #30192171 - WL#10358: SIG6 IN TEMPTABLE::HANDLER::POSITION() AT SRC/HANDLER.CC
--echo #

set sql_mode='';
SELECT DISTINCT SQL_BIG_RESULT col_json, col_char
FROM ( VALUES
  ROW( 'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx', NULL, NULL ),
  ROW( NULL, NULL, '"yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy"' )
) AS t1 ( col_char, col_time, col_json )
GROUP BY col_time ORDER BY col_json;
set sql_mode=default;

--echo #
--echo # Bug#30244935 - Sig6 in SELECT_LEX_UNIT::check_materialized_derived...
--echo #

CREATE TABLE t (col INTEGER);

SELECT t1.col
FROM (TABLE t) AS t1,
     (SELECT * FROM (VALUES ROW('Y')) AS sq_t1, t) AS t2;

DROP TABLE t;

--echo #
--echo # Bug#30273258: Sig11 in Item_values_column::is_null() ...
--echo #

CREATE TABLE t1(x VARCHAR(10));
CREATE TABLE t2(y VARCHAR(10));

INSERT INTO t1 VALUES('B');
INSERT INTO t2 VALUES('B');

SELECT 1
FROM (TABLE t1) AS table1 JOIN t2 AS table2
     ON table1.x = table2.y
WHERE table1.x IN (VALUES ROW('B'));

SELECT 1
FROM (TABLE t1) AS table1 JOIN t2 AS table2
     ON table1.x = table2.y
WHERE table1.x IN (VALUES ROW(NULL));

SELECT 1
FROM (TABLE t1) AS table1 JOIN t2 AS table2
     ON table1.x = table2.y
WHERE table1.x IN (VALUES ROW(NULL), ROW('B'));

DROP TABLE t1, t2;

--echo #
--echo # Bug#30602659: TABLE VALUE CONSTRUCTOR IGNORES LIMIT CLAUSE
--echo #

# Table value constructor syntax tests:

VALUES ROW(1), ROW(2) LIMIT 1;
(VALUES ROW(1), ROW(2), ROW(3), ROW(4), ROW(5)) LIMIT 2 OFFSET 3;

# Explicit table syntax coverage test:

CREATE TABLE t1 VALUES ROW(1), ROW(2), ROW(3), ROW(4), ROW(5);
TABLE t1 LIMIT 5 OFFSET 5;
DROP TABLE t1;

--echo # Bug#31387510: Error with VALUES() query

# note: Some ordering results with multiple rows are invalid in the old
#       optimizer. The hypergraph optimizer orders them correctly.
#       See bug#31949057 for details.

VALUES ROW(1) ORDER BY 1;
VALUES ROW(1) ORDER BY 1 DESC;
(VALUES ROW(1)) ORDER BY 1;
(VALUES ROW(1)) ORDER BY 1 DESC;

VALUES ROW(1),ROW(2) ORDER BY 1;
# Wrong ordering with the old optimizer.
VALUES ROW(1),ROW(2) ORDER BY 1 DESC;
(VALUES ROW(1),ROW(2)) ORDER BY 1;
# Wrong ordering with the old optimizer.
(VALUES ROW(1),ROW(2)) ORDER BY 1 DESC;

# Wrong ordering with the old optimizer.
VALUES ROW(1,9),ROW(2,4) ORDER BY 2;
VALUES ROW(1,9),ROW(2,4) ORDER BY 2 DESC;
# Wrong ordering with the old optimizer.
(VALUES ROW(1,9),ROW(2,4)) ORDER BY 2;
(VALUES ROW(1,9),ROW(2,4)) ORDER BY 2 DESC;

VALUES ROW(1) ORDER BY '1';
VALUES ROW(1) ORDER BY '1' DESC;
(VALUES ROW(1)) ORDER BY '1';
(VALUES ROW(1)) ORDER BY '1' DESC;

VALUES ROW(1),ROW(2) ORDER BY '1';
VALUES ROW(1),ROW(2) ORDER BY '1' DESC;
(VALUES ROW(1),ROW(2)) ORDER BY '1';
(VALUES ROW(1),ROW(2)) ORDER BY '1' DESC;

VALUES ROW(1,9),ROW(2,4) ORDER BY '2';
VALUES ROW(1,9),ROW(2,4) ORDER BY '2' DESC;
(VALUES ROW(1,9),ROW(2,4)) ORDER BY '2';
(VALUES ROW(1,9),ROW(2,4)) ORDER BY '2' DESC;

VALUES ROW(1),ROW(2) ORDER BY 1;
# Wrong ordering with the old optimizer.
VALUES ROW(1),ROW(2) ORDER BY 1 DESC;

VALUES ROW(1) ORDER BY (SELECT 1);
VALUES ROW(1) ORDER BY (SELECT 1) DESC;
(VALUES ROW(1)) ORDER BY (SELECT 1);
(VALUES ROW(1)) ORDER BY (SELECT 1) DESC;

VALUES ROW(1),ROW(2) ORDER BY (SELECT 1);
VALUES ROW(1),ROW(2) ORDER BY (SELECT 1) DESC;
(VALUES ROW(1),ROW(2)) ORDER BY (SELECT 1);
(VALUES ROW(1),ROW(2)) ORDER BY (SELECT 1) DESC;

VALUES ROW(1,9),ROW(2,4) ORDER BY (SELECT 2);
VALUES ROW(1,9),ROW(2,4) ORDER BY (SELECT 2) DESC;
(VALUES ROW(1,9),ROW(2,4)) ORDER BY (SELECT 2);
(VALUES ROW(1,9),ROW(2,4)) ORDER BY (SELECT 2) DESC;

VALUES ROW(1) ORDER BY column_0;
VALUES ROW(1) ORDER BY column_0 DESC;
(VALUES ROW(1),ROW(2)) ORDER BY column_0;
# Wrong ordering with the old optimizer.
(VALUES ROW(1),ROW(2)) ORDER BY column_0 DESC;

VALUES ROW(1),ROW(2) ORDER BY column_0;
# Wrong ordering with the old optimizer.
VALUES ROW(1),ROW(2) ORDER BY column_0 DESC;
(VALUES ROW(1),ROW(2)) ORDER BY column_0;
# Wrong ordering with the old optimizer.
(VALUES ROW(1),ROW(2)) ORDER BY column_0 DESC;

# Wrong ordering with the old optimizer.
VALUES ROW(1,9),ROW(2,4) ORDER BY column_1;
VALUES ROW(1,9),ROW(2,4) ORDER BY column_1 DESC;
# Wrong ordering with the old optimizer.
(VALUES ROW(1,9),ROW(2,4)) ORDER BY column_1;
(VALUES ROW(1,9),ROW(2,4)) ORDER BY column_1 DESC;

VALUES ROW(1) ORDER BY (SELECT column_0);
VALUES ROW(1) ORDER BY (SELECT column_0) DESC;
(VALUES ROW(1),ROW(2)) ORDER BY (SELECT column_0);
# Wrong ordering with the old optimizer.
(VALUES ROW(1),ROW(2)) ORDER BY (SELECT column_0) DESC;

VALUES ROW(1),ROW(2) ORDER BY (SELECT column_0);
# Wrong ordering with the old optimizer.
VALUES ROW(1),ROW(2) ORDER BY (SELECT column_0) DESC;
(VALUES ROW(1),ROW(2)) ORDER BY (SELECT column_0);
# Wrong ordering with the old optimizer.
(VALUES ROW(1),ROW(2)) ORDER BY (SELECT column_0) DESC;

# Wrong ordering with the old optimizer.
VALUES ROW(1,9),ROW(2,4) ORDER BY (SELECT column_1);
VALUES ROW(1,9),ROW(2,4) ORDER BY (SELECT column_1) DESC;
# Wrong ordering with the old optimizer.
(VALUES ROW(1,9),ROW(2,4)) ORDER BY (SELECT column_1);
(VALUES ROW(1,9),ROW(2,4)) ORDER BY (SELECT column_1) DESC;

--echo # Bug#32783943: Item_field::fix_outer_field: assertion `cur_query_block

CREATE TABLE t (w INTEGER);
--error ER_BAD_FIELD_ERROR
SELECT (VALUES ROW(1) ORDER BY t) AS a FROM t;
SELECT (VALUES ROW(1) ORDER BY w) AS a FROM t;
DROP TABLE t;


--echo #
--echo # BUG#32858783 ASSERTION `NULLPTR != DYNAMIC_CAST<TARGET>(ARG)' FAILED.
--echo #

CREATE TABLE t1(a INT);
CREATE VIEW v1 AS SELECT a FROM t1;
INSERT INTO v1 VALUES() AS c(a);
INSERT INTO v1 VALUES(5) AS c(a);
SELECT * FROM v1;
DROP VIEW v1;
DROP TABLE t1;

--echo #
--echo # Bug#35363550 Confusing time values
--echo #

# First row's single column's value should be NULL since WHEN clause
# doesn't match and the CASE has no ELSE. Server asserted in debug
# builds and gave wrong results in release builds
VALUES ROW(CASE 1 WHEN 2
           THEN
              TIME '0:0:0.3'
           END),
       ROW(TIME '0:0:0.2');

VALUES ROW(CASE 1 WHEN 2
           THEN
               TIME '0:0:0'
           END),
       ROW(TIME '0:0:0');

# Last row's single column's value was NULL before fix.
VALUES ROW(NULL),
       ROW(TIME '-719:0:0.2323049640');

--echo #
--echo # Bug#35512099: hypergraph_optimizer SEGV
--echo #

CREATE TABLE t(x INT);
INSERT INTO t VALUES (0);
let $query =
  INSERT INTO t VALUES ROW(1), ROW(2) UNION ALL VALUES ROW(2), ROW(3);
--replace_regex $elide_costs
--eval EXPLAIN FORMAT=TREE $query
--eval $query
--sorted_result
SELECT * FROM t;
DROP TABLE t;

--echo #
--echo # Bug#35087820: VALUES Statement with dependent subquery is wrong
--echo #

CREATE TABLE t(
  id INTEGER PRIMARY KEY,
  a VARCHAR(4),
  b VARCHAR(4),
  c VARCHAR(3));
INSERT INTO t VALUES (1, 'a1', 'b1', 'c1'), (2, 'a2', 'b2', 'c2');

--sorted_result
SELECT
  id,
  (SELECT MAX(col1) FROM (VALUES ROW(a), ROW(b), ROW(c)) AS x(col1)) AS max
FROM t;

DROP TABLE t;

--echo #
--echo # Bug#34852090: Incorrect result with VALUES in
--echo #               a correlated LATERAL subquery
--echo #

WITH v1(x) AS (VALUES ROW (1), ROW (2), ROW (3))
SELECT * FROM v1, LATERAL (VALUES ROW(v1.x)) AS v2;

--echo #
--echo # Bug#30775369: WRONG RESULT WITH "IN" COMPARATOR AND
--echo #               A TABLE VALUE CONSTRUCTOR
--echo #

CREATE TABLE t(a INT, b INT);
INSERT INTO t VALUES (1, 2), (3, 4), (5, 6), (7, 8), (9, 10), (NULL, NULL);
ANALYZE TABLE t;
# The query plan used to lack one of the filter conditions and return
# too many rows.
--let $query = SELECT * FROM t WHERE a IN (VALUES ROW(5), ROW(55), ROW(NULL))
--replace_regex $elide_costs
--eval EXPLAIN FORMAT=TREE $query
--eval $query
DROP TABLE t;

--echo #
--echo # Bug#35785452: Table value constructor in subquery:
--echo #               Assertion `!sl->order_list.first' failed.
--echo #

CREATE TABLE t(x INT);
INSERT INTO t VALUES (1), (2), (3), (4);
--sorted_result
SELECT * FROM t WHERE x >= ALL (VALUES ROW(2), ROW(3) ORDER BY column_0);
DROP TABLE t;
