#!/bin/bash

die() {
        printf -- "${1}\n"
	exit 2
}

print_usage() {
	local help="$1"

	printf "usage: $PROG_NAME [-hz] [-o <file>] <directory>\n"
	if [ "$help" != "" ]; then
		printf "\t-h  print this help screen\n"
		printf "\t-o  print playlist to <file>\n"
		printf "\t-z  shuffle entries in playlist\n"
	fi
}

parse_arguments() {
	local args=$(getopt ho:z ${*})
	[ $? != 0 ] && exit 1
	if [ $# -lt 1 ]; then
		print_usage
		exit 1
	fi
	set -- $args
	while [ $# -ge 0 ]; do
		case "$1" in
		-h) print_usage "help"; exit 0;;
		-o) ARG_OUTFILE=$2; shift; shift;;
		-z) ARG_SHUFFLE=1; shift;;
		--) shift; break;;
		esac
	done

	if [ $# -lt 1 ]; then
		die "aborting, directory missing."
	fi

	ARG_DIR=$@
}

write() {
	if [ "$ARG_OUTFILE" != "" ]; then
		printf "${1}" >> $ARG_OUTFILE
	else
		printf "${1}"
	fi
}

generate_playlist() {
	write "<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\">\n\t<trackList>\n"
	while read file; do
		local path=$(readlink -f "$file")
		write "\t\t<track><location>file://${path}</location></track>\n"
	done
	write "\t</trackList>\n</playlist>\n"
}

main() {
	parse_arguments "$@"

	local order=
	[ $ARG_SHUFFLE -eq 1 ] && order=shuf || order=sort

	find "$ARG_DIR" -name '*.aac'  \
	             -o -name '*.flac' \
	             -o -name '*.mp3'  \
	             -o -name '*.ogg'  \
	             -o -name '*.wav'  \
		| $order | generate_playlist

	exit 0
}

PROG_NAME=$(basename $0)
ARG_DIR=
ARG_OUTFILE=
ARG_SHUFFLE=0

main "$@"

exit 0

# End of file
