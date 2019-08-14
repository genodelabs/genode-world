/**
 * \brief  Test basic libfdt functionality
 * \author Sebastian Sumpf
 * \date   2019-08-09
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

extern "C" {
#include <libfdt.h>
}

#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <util/endian.h>

using namespace Genode;

enum { MAX_FDT = 128*1024 };
static char fdt[MAX_FDT];


static void memory(int offset)
{
	log("Address cells: ", fdt_address_cells(fdt, offset));
	log("Size cells ", fdt_size_cells(fdt, offset));
	log("Reg subnode offset: ", fdt_subnode_offset(fdt, offset, "reg"));

	int pr_off;
	fdt_for_each_property_offset(pr_off, fdt, offset) {

		int len;
		struct fdt_property const *pr = fdt_get_property_by_offset(fdt, pr_off, &len);
		log("Property length: ", len);
		log("name '", fdt_get_string(fdt, host_to_big_endian(pr->nameoff), nullptr), "' ",
		   " tag: ", Hex(host_to_big_endian(pr->tag)),
		   " length: ", host_to_big_endian(pr->len),
		   " name offset: ", Hex(host_to_big_endian(pr->nameoff)));

		if (strcmp(fdt_get_string(fdt, host_to_big_endian(pr->nameoff), nullptr), "reg") != 0) continue;
		unsigned *data = (unsigned *)pr->data;
		log("Poperty: ", pr,  " data at ", data);
		for (int i = 0; i < 4; i++) {
			log("[", i, "] data: ", Hex(host_to_big_endian(data[i])));
		}

		log("Set property");
		data[3] = host_to_big_endian(0xb0000000);

		for (int i = 0; i < 4; i++) {
			log("[", i, "] data: ", Hex(host_to_big_endian(data[i])));
		}
	}
}


void Component::construct(Genode::Env &env)
{
	Attached_rom_dataspace dtb { env, "tree.dtb" };

	log("Opened 'tree.dtb'");
	Genode::memcpy(fdt, dtb.local_addr<void const *>(), dtb.size());

	log("Copied ", dtb.size(), " bytes");
	log("Tree is ", fdt_check_header(fdt) == 0 ? "valid" : "corrupted");

	int offset = fdt_next_node(fdt, -1, nullptr);
	int last_offset = offset;
	while (offset >= 0) {
		char const *name  = fdt_get_name(fdt, offset, nullptr);
		log("node: ", name);

		if (strcmp(name, "memory", 6) == 0) {
			warning("Found memory");
			memory(offset);
		}

		if (strcmp(name, "ethernet", 8) == 0) {
			warning("Deleting ethernet");
			fdt_del_node(fdt, offset);
			offset = last_offset;
		}

		if (strcmp(name, "audio-codec", 11) == 0) {
			warning("Deleting audio's 'compatible' porpery");
			fdt_delprop(fdt, offset,"compatible");
		}

		last_offset = offset;
		offset = fdt_next_node(fdt, offset, nullptr);
	}

	fdt_pack(fdt);
	warning("Total size after pack: ", fdt_totalsize(fdt), " bytes");
}
