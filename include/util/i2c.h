/*
 * \brief  Type-safe, fine-grained access to I2C devices
 * \author Johannes Schlatow
 * \date   2016-12-09
 */

/*
 * Copyright (C) 2011-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__UTIL__I2C_H_
#define _INCLUDE__UTIL__I2C_H_

/* Genode includes */
#include <util/register_set.h>

namespace Genode {

	class I2c_driver_base;
	template<typename DRIVER, unsigned REG_ADDR_SIZE> class I2c_plain_access;
	template<typename DRIVER, unsigned REG_ADDR_SIZE> class I2c;
}

/**
 * Base class for I2C drivers
 */
class Genode::I2c_driver_base
{
	public:
		int write_bytes(uint8_t slaveAddr, uint8_t *msgPtr, int byteCount, bool hold=false);
		int read_byte(uint8_t slaveAddr, uint8_t *msgPtr);
};

/**
 * Plain access implementation for I2C
 */
template<typename DRIVER, unsigned REG_ADDR_SIZE>
class Genode::I2c_plain_access
{
	friend Register_set_plain_access;

	private:

		addr_t const _slave;
		DRIVER &_driver;

		/**
		 * Write 'ACCESS_T' typed 'value' to I2C slave register at 'reg'
		 */
		template <typename ACCESS_T>
		inline void _write(off_t const reg, ACCESS_T const value)
		{
			Genode::uint8_t buf[REG_ADDR_SIZE + sizeof(ACCESS_T)];

			/* write 'reg' into byte buffer using little endian */
			for (size_t i=0; i < REG_ADDR_SIZE; i++) {
				buf[i] = reg >> i*8;
			}
			/* write 'value' into byte buffer using little endian */
			for (size_t i=0; i < sizeof(ACCESS_T); i++) {
				buf[i+REG_ADDR_SIZE] = value >> i*8;
			}

			/* FIXME how to deal with errors? */
			_driver.write_bytes(_slave, buf, REG_ADDR_SIZE+sizeof(ACCESS_T)) ;
		}

		/**
		 * Read 'ACCESS_T' typed from I2C slave register at 'reg'
		 */
		template <typename ACCESS_T>
		inline ACCESS_T _read(off_t const &reg) const
		{
			Genode::uint8_t buf[REG_ADDR_SIZE];
			/* write 'reg' into byte buffer using big endian */
			for (size_t i=0; i < REG_ADDR_SIZE; i++) {
				buf[i] = reg >> i*8;
			}

			/* FIXME how to deal with errors? */
			_driver.write_bytes(_slave, buf, REG_ADDR_SIZE, true);

			/* read data into buffer */
			Genode::uint8_t read_buf[sizeof(ACCESS_T)];
			_driver.read_bytes(_slave, read_buf, sizeof(ACCESS_T));

			/* convert buffer into ACCESS_T */
			ACCESS_T value = 0;
			for (size_t i=0; i < sizeof(ACCESS_T); i++) {
				value |= read_buf[i] << i*8;
			}
			return value;
		}

		/**
		 * Read byte I2C slave register at 'reg'
		 */
		inline uint8_t _read(off_t const &reg) const
		{
			Genode::uint8_t buf[REG_ADDR_SIZE];
			/* write 'reg' into byte buffer using big endian */
			for (size_t i=0; i < REG_ADDR_SIZE; i++) {
				buf[i] = reg >> i*8;
			}

			/* FIXME how to deal with errors? */
			_driver.write_bytes(_slave, buf, REG_ADDR_SIZE, true);

			uint8_t value;
			_driver.read_bytes(_slave, &value, 1);
			return value;
		}

	public:

		/**
		 * Constructor
		 *
		 * \param slave  address of targeted I2C slave
		 */
		I2c_plain_access(addr_t const slave, DRIVER &driver) : _slave(slave), _driver(driver) { }
};


/**
 * Type-safe, fine-grained access to a I2C devices
 *
 * For further details refer to the documentation of the 'Register_set' class.
 */
template <typename DRIVER, unsigned REG_ADDR_SIZE>
struct Genode::I2c : I2c_plain_access<DRIVER, REG_ADDR_SIZE>, Register_set<I2c_plain_access<DRIVER, REG_ADDR_SIZE>>
{
	/**
	 * Constructor
	 *
	 * \param slave  slave address of targeted I2C device
	 */
	I2c(addr_t const slave, DRIVER &driver)
	: 
		I2c_plain_access<DRIVER, REG_ADDR_SIZE>(slave, driver),
	   Register_set<I2c_plain_access<DRIVER,REG_ADDR_SIZE>>(*static_cast<I2c_plain_access<DRIVER,REG_ADDR_SIZE> *>(this)) { }
};

#endif /* _INCLUDE__UTIL__I2C_H_ */
