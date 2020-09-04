#/bin/sh

echo "This is a message for Bacula" >&2

# restore mode
if [ $# -gt 0 ]; then
    echo "This is an informative message"
    read a
    read b
    echo "This is an error message" >&2
    read c
    sleep 2
    echo $a
    echo $b
    echo $c
    echo "Last line for Bacula" >&2
    exit $1
else
    echo 1
    echo 2
    echo "An other line for Bacula" >&2
    echo 3
    exit 0
fi
