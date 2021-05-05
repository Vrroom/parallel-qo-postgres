cd install/bin
./initdb -D data
./pg_ctl -D data -l logfile start
./createdb pagila
echo "Creating Toy Database ..."
./psql -d pagila -f ../../pagila/pagila-schema.sql >/dev/null 2>&1
./psql -d pagila -f ../../pagila/pagila-data.sql >/dev/null 2>&1
./psql -d pagila -f ../../pagila/pagila-insert-data.sql >/dev/null 2>&1
echo "Done!"
