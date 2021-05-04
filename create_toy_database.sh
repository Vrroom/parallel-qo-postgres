cd install/bin
./initdb -D data
./pg_ctl -D data -l logfile start
./createdb pagila
./psql -d pagila -f ../../pagila/pagila-schema.sql
./psql -d pagila -f ../../pagila/pagila-data.sql
./psql -d pagila -f ../../pagila/pagila-insert-data.sql
