/*
 * \brief  SQLite VFS layer for Genode
 * \author Emery Hemingway
 * \date   2015-01-31
 *
 * In this file VFS can refer to both the SQLite VFS and Genode VFS.
 * See http://sqlite.org/vfs.html for more information.
 *
 * This implementation of sqlite3_io_methods only meets the first
 * version of that interface. Implementing the shared memory I/O
 * methods of the second version should increase performance.
 * https://www.sqlite.org/c3ref/io_methods.html
 */

 /*
  * Copyright (C) 2016 Genode Labs GmbH
  *
  * This file is part of the Genode OS framework, which is distributed
  * under the terms of the GNU General Public License version 2.
  */

/* Genode includes */
#include <vfs/dir_file_system.h>
#include <rtc_session/connection.h>
#include <timer_session/connection.h>
#include <os/path.h>
#include <os/config.h>
#include <base/env.h>
#include <base/printf.h>
#include <util/string.h>
#include <base/lock_guard.h>

/* jitterentropy includes */
namespace Jitter {
extern "C" {
#include <jitterentropy.h>
}
}

/* Use string operations without qualifier. */
using namespace Genode;

namespace Sqlite {

/* Sqlite includes */
extern "C" {
#include <sqlite3.h>
}

#define NOT_IMPLEMENTED PWRN("Sqlite::%s not implemented", __func__);

/*******************
 ** Time and date **
 *******************/

static Timer::Connection _timer;

/*
 * Sleep for at least nMicro microseconds. Return the (approximate) number
 * of microseconds slept for.
 */
static int genode_sleep(sqlite3_vfs *pVfs, int nMicro)
{
	unsigned long then, now;

	then = _timer.elapsed_ms();
	_timer.usleep(nMicro);
	now = _timer.elapsed_ms();

	return (now - then) / 1000;
}

/**
 * Convert an Rtc::Timestamp to a Julian Day Number.
 *
 * \param   ts   Timestamp to convert.
 *
 * \return       Julian Day Number, rounded down.
 *
 * The Julian Day starts at noon and this function rounds down,
 * so the return value is effectively 12 hours behind.
 *
 * https://en.wikipedia.org/wiki/Julian_day#Calculation
 */
static unsigned julian_day(Rtc::Timestamp ts)
{
	unsigned a = (14 - ts.month) / 12;
	unsigned y = ts.year + 4800 - a;
	unsigned m = ts.month + 12*a - 3;

	return ts.day + (153*m + 2)/5 + 365*y + y/4 - y/100 + y/400 - 32046;
}

/*
 * Write into *pTime the current time and date as a Julian Day
 * number times 86400000. In other words, write into *piTime
 * the number of milliseconds since the Julian epoch of noon in
 * Greenwich on November 24, 4714 B.C according to the proleptic
 * Gregorian calendar.
 *
 * JULIAN DATE IS NOT THE SAME AS THE JULIAN CALENDAR,
 * NOR WAS IT DEVISED BY SOMEONE NAMED JULIAN.
 * THIS MIGHT BE CONFUSING.
 */
static int genode_current_time(sqlite3_vfs *pVfs, double *pTime)
{
	try {
		Rtc::Connection rtc;
		Rtc::Timestamp ts = rtc.current_time();

		*pTime = julian_day(ts)  * 86400000.0;
		*pTime += (ts.hour + 12) *   360000.0;
		*pTime += ts.minute      *    60000.0;
		*pTime += ts.second      *     1000.0;
		*pTime += ts.microsecond /     1000.0;
	} catch (...) {
		*pTime = _timer.elapsed_ms();
	}

	return SQLITE_OK;
}

/* See above. */
static int genode_current_time_int64(sqlite3_vfs *pVfs, sqlite3_int64 *pTime)
{
	try {
		Rtc::Connection rtc;
		Rtc::Timestamp ts = rtc.current_time();

		*pTime = julian_day(ts)  * 86400000;
		*pTime += (ts.hour + 12) *   360000;
		*pTime += ts.minute      *    60000;
		*pTime += ts.second      *     1000;
		*pTime += ts.microsecond /     1000;
	} catch (...) {
		*pTime = _timer.elapsed_ms();
	}

	return SQLITE_OK;
}


/**********************
 ** Shared libraries **
 **********************/

/*
 * The following four VFS methods:
 *
 *   xDlOpen
 *   xDlError
 *   xDlSym
 *   xDlClose
 *
 * are supposed to implement the functionality needed by SQLite to load
 * extensions compiled as shared objects. This simple VFS does not support
 * this functionality, so the following functions are no-ops.
 */
static void *genode_dl_open(sqlite3_vfs *pVfs, const char *zPath)
{
	NOT_IMPLEMENTED
	return 0;
}
static void genode_dl_error(sqlite3_vfs *pVfs, int nByte, char *zErrMsg)
{
	NOT_IMPLEMENTED
	sqlite3_snprintf(nByte, zErrMsg, "Loadable extensions are not implemented");
	zErrMsg[nByte-1] = '\0';
}
static void (*genode_dl_sym(sqlite3_vfs *pVfs, void *pH, const char *z))(void)
{
	NOT_IMPLEMENTED
	return 0;
}
static void genode_dl_close(sqlite3_vfs *pVfs, void *pHandle)
{
	NOT_IMPLEMENTED
	return;
}


/************
 ** Random **
 ************/

static Jitter::rand_data *_jitter;

static int genode_randomness(sqlite3_vfs *pVfs, int len, char *buf)
{
	static Genode::Lock jitter_lock;
	Genode::Lock::Guard guard(jitter_lock);

	return Jitter::jent_read_entropy(_jitter, buf, len);
}

static int random_string(char *buf, int len)
{
	const unsigned char chars[] =
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"0123456789";

	if (genode_randomness(0, len, buf)  != (len))
		return -1;

	for(int i=0; i <len; i++) {
		buf[i] = (char)chars[ ((unsigned char)buf[i]) % (sizeof(chars)-1) ];
	}
	return 0;
}


/*****************
 ** File system **
 *****************/

/*
** When using this VFS, the sqlite3_file* handles that SQLite uses are
** actually pointers to instances of type Genode_file.
*/
typedef struct Genode_file Genode_file;
struct Genode_file {
	sqlite3_file          base;  /* Base class. Must be first. */
	Vfs::Dir_file_system *vfs;
	Vfs::Vfs_handle      *handle;
	char                  path[Vfs::MAX_PATH_LEN];
	bool                  delete_on_close;
};


static int genode_delete(sqlite3_vfs *pVfs, const char *path, int dirSync)
{
	Vfs::Dir_file_system *vfs = (Vfs::Dir_file_system*)pVfs->pAppData;

	typedef Vfs::Directory_service::Unlink_result Result;
	switch (vfs->unlink(path)) {
	case Result::UNLINK_ERR_NO_ENTRY:  return SQLITE_IOERR_DELETE_NOENT;
	case Result::UNLINK_ERR_NO_PERM:   return SQLITE_IOERR_ACCESS;
	case Result::UNLINK_ERR_NOT_EMPTY: return SQLITE_IOERR_DELETE;
	case Result::UNLINK_OK: break;
	}

	if (dirSync) {
		Path<Vfs::MAX_PATH_LEN> dir_path(path);
		dir_path.strip_last_element();
		dir_path.remove_trailing('/');
		vfs->sync(dir_path.base());
	}

	return SQLITE_OK;
}


static int genode_close(sqlite3_file *pFile)
{
	Genode_file *p = (Genode_file*)pFile;
	{ Vfs::Vfs_handle::Guard guard(p->handle); }

	typedef Vfs::Directory_service::Unlink_result Result;
	if (p->delete_on_close) switch(p->vfs->unlink(p->path)) {
		case Result::UNLINK_ERR_NO_ENTRY:  return SQLITE_IOERR_DELETE_NOENT;
		case Result::UNLINK_ERR_NO_PERM:   return SQLITE_IOERR_ACCESS;
		case Result::UNLINK_ERR_NOT_EMPTY: return SQLITE_IOERR_DELETE;
		case Result::UNLINK_OK: break;
	}
	return SQLITE_OK;
}


/*
 * Reads and writes assume that SQLite will not operate
 * over more than 4096 bytes at a time, and likely only 512.
 * https://www.sqlite.org/atomiccommit.html#hardware
 *
 * These next two methods are not thread safe.
 */


static int genode_write(sqlite3_file *pFile, const void *buf, int count, sqlite_int64 offset)
{
	Vfs::file_size n;
	Genode_file *p = (Genode_file*)pFile;

	p->handle->seek(offset);

	typedef Vfs::File_io_service::Write_result Result;
	switch (p->handle->fs().write(p->handle, (char const *)buf, count, n)) {
	case Result::WRITE_ERR_AGAIN:
	case Result::WRITE_ERR_WOULD_BLOCK:
	case Result::WRITE_ERR_INVALID:
	case Result::WRITE_ERR_IO:
	case Result::WRITE_ERR_INTERRUPT:
		return SQLITE_IOERR_WRITE;
	case Result::WRITE_OK: break;	
	}
	return (n < Vfs::file_size(count)) ? SQLITE_IOERR_WRITE : SQLITE_OK;
}


static int genode_read(sqlite3_file *pFile, void *buf, int count, sqlite_int64 offset)
{
	Vfs::file_size n;
	Genode_file *p = (Genode_file*)pFile;

	p->handle->seek(offset);

	typedef Vfs::File_io_service::Read_result Result;
	switch (p->handle->fs().read(p->handle, (char *)buf, count, n)) {
	case Result::READ_ERR_AGAIN:
	case Result::READ_ERR_WOULD_BLOCK:
	case Result::READ_ERR_INVALID:
	case Result::READ_ERR_IO:
	case Result::READ_ERR_INTERRUPT:
		return SQLITE_IOERR_READ;
	case Result::READ_OK: break;
	}
	return (n < Vfs::file_size(count)) ? SQLITE_IOERR_SHORT_READ : SQLITE_OK;
};


static int genode_truncate(sqlite3_file *pFile, sqlite_int64 size)
{
	Genode_file *p = (Genode_file*)pFile;

	typedef Vfs::File_io_service::Ftruncate_result Result;
	switch (p->handle->fs().ftruncate(p->handle, size)) {
	case Result::FTRUNCATE_ERR_NO_PERM:
	case Result::FTRUNCATE_ERR_INTERRUPT: return SQLITE_IOERR_TRUNCATE;
	case Result::FTRUNCATE_ERR_NO_SPACE:  return SQLITE_FULL;
	case Result::FTRUNCATE_OK: break;
	}
	return SQLITE_OK;
}


static int genode_sync(sqlite3_file *pFile, int flags)
{
	Genode_file *p = (Genode_file*)pFile;

	p->vfs->sync(p->path);
	return SQLITE_OK;
}


static int genode_file_size(sqlite3_file *pFile, sqlite_int64 *pSize)
{
	Genode_file *p = (Genode_file*)pFile;

	Vfs::Directory_service::Stat stat;

	typedef Vfs::Directory_service::Stat_result Result;
	switch(p->vfs->stat(p->path, stat)) {
	case Result::STAT_ERR_NO_ENTRY: return SQLITE_IOERR_FSTAT;
	case Result::STAT_ERR_NO_PERM:  return SQLITE_IOERR_ACCESS;
	case Result::STAT_OK: break;
	}
	*pSize = stat.size;
	return SQLITE_OK;
}


static int genode_lock(sqlite3_file *pFile, int eLock)
{
	NOT_IMPLEMENTED
	return SQLITE_OK;
}
static int genode_unlock(sqlite3_file *pFile, int eLock)
{
	NOT_IMPLEMENTED
	return SQLITE_OK;
}
static int genode_check_reserved_lock(sqlite3_file *pFile, int *pResOut)
{
	NOT_IMPLEMENTED
	*pResOut = 0;
	return SQLITE_OK;
}


/*
 * No xFileControl() verbs are implemented by this VFS.
 * Without greater control over writing, there isn't utility in processing this.
 * See https://www.sqlite.org/c3ref/c_fcntl_busyhandler.html
*/
static int genode_file_control(sqlite3_file *pFile, int op, void *pArg) { return SQLITE_OK; }


/*
 * The xSectorSize() and xDeviceCharacteristics() methods. These two
 * may return special values allowing SQLite to optimize file-system
 * access to some extent. But it is also safe to simply return 0.
 */
static int genode_sector_size(sqlite3_file *pFile) { return 0; }


static int genode_device_characteristics(sqlite3_file *pFile) { return 0; }


static int genode_access(sqlite3_vfs *pVfs, const char *path, int flags, int *pResOut)
{
	Vfs::Dir_file_system *vfs = (Vfs::Dir_file_system*)pVfs->pAppData;

	unsigned mode = flags == SQLITE_ACCESS_READWRITE
		? Vfs::Directory_service::OPEN_MODE_RDWR
		: Vfs::Directory_service::OPEN_MODE_RDONLY;

	Vfs::Vfs_handle *handle = nullptr;

	typedef Vfs::Directory_service::Open_result Result;
	if (vfs->open(path, mode, &handle) == Result::OPEN_OK) {
		*pResOut = true;
		Vfs::Vfs_handle::Guard guard(handle);
	} else
		*pResOut = false;

	return SQLITE_OK;
}


static int genode_full_pathname(sqlite3_vfs *pVfs, const char *path_in, int out_len, char *path_out)
{
	/*
	 * No support for current working directory, work from /.
	 */
	Path<Vfs::MAX_PATH_LEN> path(path_in);
	strncpy(path_out, path.base(), out_len);
	return SQLITE_OK;
}


static int genode_open(
		sqlite3_vfs *pVfs,
		const char *name,               /* File to open, or 0 for a temp file */
		sqlite3_file *pFile,            /* Pointer to Genode_file struct to populate */
		int flags,                      /* Input SQLITE_OPEN_XXX flags */
		int *pOutFlags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
	)
{
	static const sqlite3_io_methods genodeio = {
		1,                               /* iVersion */
		genode_close,                    /* xClose */
		genode_read,                     /* xRead */
		genode_write,                    /* xWrite */
		genode_truncate,                 /* xTruncate */
		genode_sync,                     /* xSync */
		genode_file_size,                /* xFileSize */
		genode_lock,                     /* xLock */
		genode_unlock,                   /* xUnlock */
		genode_check_reserved_lock,      /* xCheckReservedLock */
		genode_file_control,             /* xFileControl */
		genode_sector_size,              /* xSectorSize */
		genode_device_characteristics    /* xDeviceCharacteristics */
	};

	Vfs::Dir_file_system *vfs = (Vfs::Dir_file_system*)pVfs->pAppData;
	Genode_file *p = (Genode_file*)pFile;
	memset(p, 0x00, sizeof(Genode_file));

	if (!name) {
		#define TEMP_PREFIX "sqlite_"
		#define TEMP_LEN 24

		char *s = &p->path[0];
		strncpy(s, TEMP_PREFIX, sizeof(TEMP_PREFIX));

		if (random_string(s + sizeof(TEMP_PREFIX)-1, TEMP_LEN-(sizeof(TEMP_PREFIX))))
			return SQLITE_ERROR;
	} else
		strncpy(p->path, name, sizeof(p->path));

	unsigned mode = flags&SQLITE_OPEN_READWRITE
		? Vfs::Directory_service::OPEN_MODE_WRONLY
		: Vfs::Directory_service::OPEN_MODE_WRONLY;

	if (flags&SQLITE_OPEN_CREATE)
		mode |= Vfs::Directory_service::OPEN_MODE_CREATE;

	typedef Vfs::Directory_service::Open_result Result;
	switch(vfs->open(p->path, mode, &p->handle)) {
	case Result::OPEN_ERR_UNACCESSIBLE:
	case Result::OPEN_ERR_NO_PERM:
	case Result::OPEN_ERR_EXISTS:
	case Result::OPEN_ERR_NAME_TOO_LONG:
	case Result::OPEN_ERR_NO_SPACE:
		return SQLITE_CANTOPEN;
	case Result::OPEN_OK: break;
	}

	p->base.pMethods = &genodeio;
	p->vfs = vfs;
	p->delete_on_close = flags&SQLITE_OPEN_DELETEONCLOSE;

	/* Ignore flags. */
	if (pOutFlags)
		*pOutFlags = flags
			& (SQLITE_OPEN_DELETEONCLOSE |
		       SQLITE_OPEN_READWRITE   |
		       SQLITE_OPEN_READONLY  |
		       SQLITE_OPEN_CREATE);

	return SQLITE_OK;
}



/*****************************************
  ** Library initialization and cleanup **
  ****************************************/

#define VFS_NAME "genode_vfs"

int sqlite3_os_init(void)
{
	Genode::log("initializing native SQLite OS interface");

	int ret = Jitter::jent_entropy_init();
	if(ret) {
		Genode::error("Jitter entropy initialization failed with error code ", ret);
		return SQLITE_ERROR;
	}

	_jitter = Jitter::jent_entropy_collector_alloc(0, 0);
	if (!_jitter) {
		Genode::error("Jitter entropy collector initialization failed");
		return SQLITE_ERROR;
	}

	Vfs::Dir_file_system *genode_vfs = nullptr;
	try {
		Xml_node node = Genode::config()->xml_node().sub_node("sqlite").sub_node("vfs");
		genode_vfs = new (Genode::env()->heap())
			Vfs::Dir_file_system(node, Vfs::global_file_system_factory());

	} catch (Genode::Xml_node::Nonexistent_sub_node) {
		try {
			Xml_node node = Genode::config()->xml_node().sub_node("vfs");
			genode_vfs = new (Genode::env()->heap())
				Vfs::Dir_file_system(node, Vfs::global_file_system_factory());
			PWRN("additional VFS created for SQLite");

		} catch (Genode::Xml_node::Nonexistent_sub_node) {
			Genode::error("no VFS defined for SQLite library");
			return SQLITE_ERROR;
		}
	} catch (...) {
		Genode::error("error loading VFS definition for SQLite library");
		return SQLITE_ERROR;
	}

	static sqlite3_vfs vfs = {
		2,                         /* iVersion */
		sizeof(Genode_file),       /* szOsFile */
		Vfs::MAX_PATH_LEN,         /* mxPathname */
		0,                         /* pNext */
		VFS_NAME,                  /* zName */
		genode_vfs,                /* pAppData */
		genode_open,               /* xOpen */
		genode_delete,             /* xDelete */
		genode_access,             /* xAccess */
		genode_full_pathname,      /* xFullPathname */
		genode_dl_open,            /* xDlOpen */
		genode_dl_error,           /* xDlError */
		genode_dl_sym,             /* xDlSym */
		genode_dl_close,           /* xDlClose */
		genode_randomness,         /* xRandomness */
		genode_sleep,              /* xSleep */
		genode_current_time,       /* xCurrentTime */
		0,                      /* xGetLastError */
		genode_current_time_int64, /* xCurrentTimeInt64 */
	};

	sqlite3_vfs_register(&vfs, 0);
	return SQLITE_OK;
}


int sqlite3_os_end(void)
{
	sqlite3_vfs *vfs = sqlite3_vfs_find(VFS_NAME);
	sqlite3_vfs_unregister(vfs);
	destroy(Genode::env()->heap(), (Vfs::Dir_file_system*)vfs->pAppData);
	Jitter::jent_entropy_collector_free(_jitter);

	return SQLITE_OK;
}

} /* namespace Sqlite */
