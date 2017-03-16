/*
 * \brief  Zynq I2C driver to be used with IO framework
 * \author Johannes Schlatow
 * \author Mark Albers
 * \date   2015-03-12
 */

/*
 * Copyright (C) 2015-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__DRIVERS__I2C_H_
#define _INCLUDE__DRIVERS__I2C_H_

/* Genode includes */
#include <util/i2c.h>
#include <util/mmio.h>
#include <base/attached_io_mem_dataspace.h>

namespace Genode {
	class I2c_driver;
}

/**
 * I2C driver for Zynq
 */
class Genode::I2c_driver : Genode::I2c_driver_base, Attached_io_mem_dataspace, Mmio
{
	private:
		enum {
			/* Maximal transfer size */
			I2C_MAX_TRANSFER_SIZE = 252,

			/* FIFO size */
			I2C_FIFO_DEPTH = 16,

			/* Number of bytes at data intr */
			I2C_DATA_INTR_DEPTH = 14,
		};

		int sendByteCount;
		uint8_t *sendBufferPtr;

		/*
		 * Registers
		 */
		struct Control : Register<0x0, 16>
		{
			struct divisor_a  : Bitfield<14,2> {};
			struct divisor_b  : Bitfield<8,6> {};
			struct CLR_FIFO   : Bitfield<6,1> {};
			struct SLVMON     : Bitfield<5,1> {};
			struct HOLD       : Bitfield<4,1> {};
			struct ACK_EN     : Bitfield<3,1> {};
			struct NEA        : Bitfield<2,1> {};
			struct MS         : Bitfield<1,1> {};
			struct RW         : Bitfield<0,1> {
				enum {
					SENDING = 0,
					RECEIVING = 1,
				};
			};
		};

		struct Status : Register<0x4, 16>
		{
			struct BA     : Bitfield<8,1> {};
			struct RXOVF  : Bitfield<7,1> {};
			struct TXDV   : Bitfield<6,1> {};
			struct RXDV   : Bitfield<5,1> {};
			struct RXRW   : Bitfield<3,1> {};
		};

		struct I2C_address : Register<0x8, 16>
		{
			struct ADD : Bitfield<0,10> {};
		};

		struct I2C_data : Register<0xC, 16>
		{
			struct DATA : Bitfield<0,8> {};
		};

		template <unsigned OFFSET, unsigned SIZE>
		struct Interrupt_register : Register<OFFSET, SIZE>
		{
			template <unsigned POS, unsigned BIT>
			using Bitfield = typename Register<OFFSET, SIZE>::template Bitfield<POS, BIT>;

			struct ARB_LOST : Bitfield<9,1> {};
			struct RX_UNF   : Bitfield<7,1> {};
			struct TX_OVF   : Bitfield<6,1> {};
			struct RX_OVF   : Bitfield<5,1> {};
			struct SLV_RDY  : Bitfield<4,1> {};
			struct TO       : Bitfield<3,1> {};
			struct NACK     : Bitfield<2,1> {};
			struct DATA     : Bitfield<1,1> {};
			struct COMP     : Bitfield<0,1> {};
		};

		struct Interrupt_status : Interrupt_register<0x10, 16> { };

		struct Transfer_size : Register<0x14, 8>
		{
			struct SIZE : Bitfield<0,8> {};
		};

		struct Slave_mon_pause : Register<0x18, 8>
		{
			struct PAUSE : Bitfield<0,4> {};
		};

		struct Time_out : Register<0x1C, 8>
		{
			struct TO : Bitfield<0,8> {};
		};

		struct Interrupt_mask : Interrupt_register<0x20, 16> { };

		struct Interrupt_enable : Interrupt_register<0x24, 16> { };

		struct Interrupt_disable : Interrupt_register<0x28, 16> { };

		/*
		 * Helper functions
		 */
		void _init()
		{
			write<Control>(Control::divisor_a::bits(2) |
					Control::divisor_b::bits(16) |
					Control::ACK_EN::bits(1) |
					Control::CLR_FIFO::bits(1) |
					Control::NEA::bits(1) |
					Control::MS::bits(1));
		}

		void _set_direction(int direction)
		{
			write<Control::CLR_FIFO>(1);
			write<Control::RW>(direction);
		}

		void _transmitFifoFill()
		{
			uint8_t availBytes;
			int loopCnt;
			int numBytesToSend;

			/*
			 * Determine number of bytes to write to FIFO.
			 */
			availBytes = I2C_FIFO_DEPTH - read<Transfer_size::SIZE>();
			numBytesToSend = sendByteCount > availBytes ? availBytes : sendByteCount;

			/*
			 * Fill FIFO with amount determined above.
			 */
			for (loopCnt = 0; loopCnt < numBytesToSend; loopCnt++) 
			{
				write<I2C_data::DATA>(*sendBufferPtr);
				sendBufferPtr++;
				sendByteCount--;
			}
		}


	public:
		I2c_driver(Genode::Env &env, Genode::addr_t const mmio_base, Genode::size_t const mmio_size) :
			Genode::Attached_io_mem_dataspace(env, mmio_base, mmio_size),
			Genode::Mmio((Genode::addr_t)local_addr<void>())
		{
			_init();
		}

		int write_bytes(uint8_t slaveAddr, uint8_t *msgPtr, int byteCount, bool hold=false);
		int read_bytes(uint8_t slaveAddr, uint8_t *msgPtr, int byteCount);
};

int Genode::I2c_driver::write_bytes(uint8_t slaveAddr, uint8_t *msgPtr, int byteCount, bool hold)
{
	uint32_t intrs, intrStatusReg;

	sendByteCount = byteCount;
	sendBufferPtr = msgPtr;

	/*
	 * Set HOLD bit if byteCount is bigger than FIFO or HOLD is forced
	 */
	if (byteCount > I2C_FIFO_DEPTH || hold) write<Control::HOLD>(1);

	/*
	 * Init sending master.
	 */
	_set_direction(Control::RW::SENDING);

	/*
	 * intrs keeps all the error-related interrupts.
	 */
	intrs = Interrupt_status::ARB_LOST::reg_mask() | 
			  Interrupt_status::TX_OVF::reg_mask() |
			  Interrupt_status::NACK::reg_mask();
	
	/*
	 * Clear the interrupt status register before use it to monitor.
	 */
	write<Interrupt_status>(read<Interrupt_status>());

	/*
	 * Transmit first FIFO full of data.
	 */
	_transmitFifoFill();
	write<I2C_address::ADD>(slaveAddr);
	intrStatusReg = read<Interrupt_status>();
		
	/*
	 * Continue sending as long as there is more data and there are no errors.
	 */
	while ((sendByteCount > 0) && ((intrStatusReg & intrs) == 0)) 
	{
		/*
		 * Wait until transmit FIFO is empty.
		 */
		if (read<Status::TXDV>() != 0) 
		{
			intrStatusReg = read<Interrupt_status>();
			continue;
		}

		/*
		 * Send more data out through transmit FIFO.
		 */
		_transmitFifoFill();
	}

	/*
	 * Check for completion of transfer.
	 */
	while ((intrStatusReg & Interrupt_status::COMP::reg_mask()) == 0)
	{

		intrStatusReg = read<Interrupt_status>();
		/*
		 * If there is an error, tell the caller.
		 */
		if ((intrStatusReg & intrs) != 0) 
		{
			return 1;
		}
	}

	if (!hold)
		write<Control::HOLD>(0);

	return 0;
}

int Genode::I2c_driver::read_bytes(uint8_t slaveAddr, uint8_t *msgPtr, int byteCount)
{
	uint32_t intrs, intrStatusReg;

	/*
	 * Init receiving master.
	 */
	_set_direction(Control::RW::RECEIVING);

	/*
	 * Clear the interrupt status register before use it to monitor.
	 */
	write<Interrupt_status>(read<Interrupt_status>());

	/*
	 * Set up the transfer size register so the slave knows how much
	 * to send to us.
	 */

	/* FIXME implement byteCount > 1 */
	if (byteCount > 1)
		Genode::error("I2C driver: byteCount > 1 not implemented");

	write<Transfer_size::SIZE>(1);

	/*
	 * Set slave address.
	 */
	write<I2C_address::ADD>(slaveAddr);

	/*
	 * intrs keeps all the error-related interrupts.
	 */
	intrs = Interrupt_status::ARB_LOST::reg_mask() | 
			  Interrupt_status::RX_OVF::reg_mask() |
			  Interrupt_status::RX_UNF::reg_mask() |
			  Interrupt_status::NACK::reg_mask();

	/*
	 * Poll the interrupt status register to find the errors.
	 */
	intrStatusReg = read<Interrupt_status>();

	while (read<Status::RXDV>() != 1)
	{
		if (intrStatusReg & intrs) return 1;
	}

	if (intrStatusReg & intrs) return 1;

	*msgPtr = read<I2C_data::DATA>();

	while (read<Interrupt_status::COMP>() != 1) {}

	return 0;
}

#endif /* _INCLUDE__DRIVERS__I2C_H_ */
