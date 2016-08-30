#!/bin/bash

die() {
        printf -- "${1}\n"
	exit 2
}

print_usage() {
	local help="$1"

	printf "usage: $PROG_NAME [-hv] [-i <file>\ [-o <file>] [-m <0..100>] [-c <0..100>]\n"
	if [ "$help" != "" ]; then
		printf "\t-c  set client volume\n"
		printf "\t-h  print this help screen\n"
		printf "\t-i  read config from <file>\n"
		printf "\t-m  set master volume\n"
		printf "\t-o  print config to <file>\n"
		printf "\t-v  be verbose, e.g. print current track\n"
	fi
}

parse_arguments() {
	local args=$(getopt hc:i:m:o:sv ${*})
	[ $? != 0 ] && exit 1
	set -- $args
	while [ $# -ge 0 ]; do
		case "$1" in
		-c) ARG_CLIENT="$2"; shift; shift;;
		-h) print_usage "help"; exit 0;;
		-i) ARG_INFILE="$2"; shift; shift;;
		-m) ARG_MASTER="$2"; shift; shift;;
		-o) ARG_OUTFILE="$2"; shift; shift;;
		-v) ARG_VERBOSE=1; shift;;
		--) shift; break;;
		esac
	done

	[ "$ARG_INFILE" = "" ] && ARG_INFILE="/config/mixer.config"
}

write() {
	if [ "$ARG_OUTFILE" != "" ]; then
		printf "${1}" >> $ARG_OUTFILE
	else
		printf "${1}"
	fi
}

generate_config() {
	if [ "$ARG_MASTER" = "" ] && [ "$ARG_CLIENT" = "" ]; then
		cat $ARG_INFILE
		return
	fi

	[ "$ARG_OUTFILE" = "$ARG_INFILE" ] && inplace="-i" || inplace=

	# XXX distinguish between master and client
	[ "$ARG_MASTER" != "" ] && \
		sed_args="-e 's/ volume=\\\".*\\\" / volume=\\\"'$ARG_MASTER'\\\" /'"
	[ "$ARG_CLIENT" != "" ] && \
		sed_args="$sed_args -e 's/ volume=\\\".*\\\" / volume=\\\"'$ARG_CLIENT'\\\" /'"

	if [ "$ARG_OUTFILE" != "" ]; then
		echo sed $sed_args $inplace ${ARG_INFILE} | /bin/bash > $ARG_OUTFILE
	else
		echo sed $sed_args $inplace ${ARG_INFILE} | /bin/bash
	fi
}

main() {
	parse_arguments "$@"

	generate_config

	exit 0
}

PROG_NAME=$(basename $0)
ARG_CLIENT=
ARG_INFILE=
ARG_MASTER=
ARG_OUTFILE=
ARG_VERBOSE=0

main "$@"

exit 0

# End of file
