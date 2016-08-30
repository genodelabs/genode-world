#!/bin/bash

die() {
        printf -- "${1}\n"
	exit 2
}

print_usage() {
	local help="$1"

	printf "usage: $PROG_NAME [-hnsv] [-o <file>]\n"
	if [ "$help" != "" ]; then
		printf "\t-h  print this help screen\n"
		printf "\t-n  play next track from playlist\n"
		printf "\t-o  print config to <file>\n"
		printf "\t-s  stop playback\n"
		printf "\t-v  be verbose, e.g. print current track\n"
	fi
}

parse_arguments() {
	local args=$(getopt hn:o:sv ${*})
	if [ $# -eq 0 ]; then
		ARG_CURRENT=1
		return
	fi

	[ $? != 0 ] && exit 1
	set -- $args
	while [ $# -ge 0 ]; do
		case "$1" in
		-h) print_usage "help"; exit 0;;
		-n) ARG_NEXT="$2"; shift; shift;;
		-o) ARG_OUTFILE="$2"; shift; shift;;
		-s) ARG_STOP=1; shift;;
		-v) ARG_VERBOSE=1; shift;;
		--) shift; break;;
		esac
	done
}

write() {
	if [ "$ARG_OUTFILE" != "" ]; then
		printf "${1}" >> $ARG_OUTFILE
	else
		printf "${1}"
	fi
}

generate_config() {
	local selected=
	[ "$ARG_NEXT" != "" ] && selected=" selected_track=\"${ARG_NEXT}\""
	local state=
	[ $ARG_STOP -eq 1 ] && state="stopped" || state="playing"

	write "<config state=\"${state}\"${selected}>\n"
	write "\t<libc> <vfs> <fs/> </vfs> </libc>\n"
	write "</config>\n"
}

show_current_track() {
	cat /reports/current_track 2> /dev/null
}

main() {
	parse_arguments "$@"

	if [ $ARG_CURRENT -eq 1 ]; then
		show_current_track
		exit 0
	fi

	generate_config

	if [ $ARG_VERBOSE -eq 1 ]; then
		sleep 1
		show_current_track
	fi

	exit 0
}

PROG_NAME=$(basename $0)
ARG_NEXT=""
ARG_CURRENT=0
ARG_SHUFFLE=0
ARG_STOP=0
ARG_VERBOSE=0

main "$@"

exit 0

# End of file
