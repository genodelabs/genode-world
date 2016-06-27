///
/// \file       main.cc
/// \author     Menno Valkema <menno.valkema@nlcsl.com>
/// \date       2016-05-17
///
/// \copyright  Copyright (C) 2016 Cyber Security Labs B.V. The Netherlands.
///             All rights reserved.
///
/// \brief      Genode fesrv component. Port of https://github.com/riscv/riscv-fesvr
///

#include "htif.h"

#include <os/attached_io_mem_dataspace.h>
#include <os/config.h>

#include <vector>
#include <cassert>

#define read_reg(r) (dev_vaddr[r])
#define write_reg(r, v) (dev_vaddr[r] = v)

class Htif_zedboard_genode : public htif_t
{
	public:
		Htif_zedboard_genode( std::string image = "image.elf" ):
			htif_t( std::vector<std::string>( { image } ) ),
		        _io_mem_dataspace( DEV_PADDR, 2 * sizeof( uintptr_t ), true ),
		        dev_vaddr( _io_mem_dataspace.local_addr<uintptr_t>() )

		{
			write_reg( 31, 0 ); // reset
		}
	protected:
		ssize_t read( void *buf, size_t max_size )
		{
			uint32_t *x = ( uint32_t * )buf;
			assert( max_size >= sizeof( *x ) );

			uintptr_t c = read_reg( 0 );
			uint32_t count = 0;

			if ( c > 0 )
			{
				for ( count=0; count<c && count*sizeof( *x )<max_size; count++ )
				{
					x[count] = read_reg( 1 );
				}
			}

			return count*sizeof( *x );
		}
		ssize_t write( const void *buf, size_t size )
		{
			const uint32_t *x = ( const uint32_t * )buf;
			assert( size >= sizeof( *x ) );

			for ( uint32_t i = 0; i < size/sizeof( *x ); i++ )
			{
				write_reg( 0, x[i] );
			}

			return size;
		}

		size_t chunk_max_size() { return 64; }
		size_t chunk_align() { return 64; }
		uint32_t mem_mb() { return 256; }
		uint32_t num_cores() { return 1; }

	private:
		Genode::Attached_io_mem_dataspace _io_mem_dataspace;
		volatile uintptr_t *dev_vaddr;

		const static uintptr_t DEV_PADDR = 0x43C00000;
};

static const std::string image_name()
{
	static const size_t LEN = 128;
	char buf[LEN];
	Genode::memset( buf, 0, LEN );

	try
	{
		Genode::config()->xml_node().sub_node( "fesrv" ).attribute( "image" ).value(
		    buf, LEN - 1 );
	}
	catch ( ... )
	{
		throw std::runtime_error( "elf-image not configured" );
	}

	return std::string( buf );
}

int main()
{
	try
	{
		auto image = image_name();
		Htif_zedboard_genode htif( image );
		PINF( "Initialized Htif_zedboard with %s", image.c_str() );
		htif.run();
	}
	catch ( const std::runtime_error &e )
	{
		PERR( "Caught runtime error: %s", e.what() );
	}
	catch ( ... )
	{
		PERR( "Unknown error occured" );
	}
}
