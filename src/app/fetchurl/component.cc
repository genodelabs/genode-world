/*
 * \brief  Native fetchurl utility for Nix
 * \author Emery Hemingway
 * \date   2016-03-08
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <timer_session/connection.h>
#include <os/path.h>
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/log.h>

/* cURL includes */
#include <curl/curl.h>

/* Libc includes */
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

struct Stats
{
	Timer::Connection &timer;
	unsigned long     next = 0;
	unsigned long     remain = 0;
	char       const *url;

	Stats(Timer::Connection &t, char const *u)
	: timer(t), url(u) { }
};

static size_t write_callback(char   *ptr,
                             size_t  size,
                             size_t  nmemb,
                             void   *userdata)
{
	int *fd = (int*)userdata;
	return write(*fd, ptr, size*nmemb);
}


int progress_callback(void   *clientp,
                      double dltotal,
                      double dlnow,
                      double ultotal,
                      double ulnow)
{
	Stats *stats = (Stats*)clientp;

	unsigned long now = stats->timer.elapsed_ms();
	unsigned long remain = dltotal-dlnow;

	if ((now > stats->next) && (remain != stats->remain)) {
		stats->next = now + 1000;
		stats->remain = remain;
		Genode::log(stats->url, ": ", remain, " bytes remain");
	}
	return CURLE_OK;
}

void Component::construct(Genode::Env &env)
{
	Genode::Attached_rom_dataspace config(env, "config");

	Timer::Connection timer(env);
	/* wait for DHCP */
	timer.msleep(4000);

	Genode::String<256> url;
	Genode::Path<256>   path;
	CURLcode res = CURLE_OK;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	Genode::Xml_node config_node = config.xml();

	bool verbose = config_node.attribute_value("verbose", false);

	config_node.for_each_sub_node("fetch", [&] (Genode::Xml_node node) {

		if (res != CURLE_OK) return;
		try {
			node.attribute("url").value(&url);
			node.attribute("path").value(path.base(), path.capacity());
		} catch (...) { Genode::error("error reading 'fetch' node"); return; }

		char const *out_path = path.base();

		int fd = open(out_path, O_CREAT | O_RDWR);
		if (fd == -1) {
			switch (errno) {
			case EACCES:
				Genode::error("permission denied at ", out_path); break;
			case EEXIST:
				Genode::error(out_path, " already exists"); break;
			case EISDIR:
				Genode::error(out_path, " is a directory"); break;
			case ENOSPC:
				Genode::error("cannot create ", out_path, ", out of space"); break;
			}
			env.parent().exit(errno);
			return;
		}

		CURL *curl = curl_easy_init();
		if (!curl) {
			Genode::error("failed to initialize libcurl");
			res = CURLE_FAILED_INIT;
			return;
		}

		Stats stats(timer, url.string());

		curl_easy_setopt(curl, CURLOPT_URL, url.string());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, true);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fd);

		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &stats);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		/* check for optional proxy configuration */
		try {
			Genode::String<256> proxy;
			node.attribute("proxy").value(&proxy);
			curl_easy_setopt(curl, CURLOPT_PROXY, proxy.string());
		} catch (...) { }

		res = curl_easy_perform(curl);
		close(fd);
		if (res != CURLE_OK)
			Genode::error(curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	});

	curl_global_cleanup();

	Genode::warning("SSL certificates not verified");

	env.parent().exit(res ^ CURLE_OK);
}

