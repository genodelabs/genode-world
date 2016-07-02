///
/// \file       main.cc
/// \author     Menno Valkema <menno.valkema@nlcsl.com>
/// \date       2016-05-17
///
/// \brief      Genode fesrv component. Port of https://github.com/riscv/riscv-fesvr
///

/// 
/// Copyright (c) 2016, Cyber Security Labs B.V.
/// All rights reserved.
/// 
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
/// 1. Redistributions of source code must retain the above copyright
///    notice, this list of conditions and the following disclaimer.
/// 2. Redistributions in binary form must reproduce the above copyright
///    notice, this list of conditions and the following disclaimer in the
///    documentation and/or other materials provided with the distribution.
/// 3. All advertising materials mentioning features or use of this software
///    must display the following acknowledgement:
///    This product includes software developed by the Cyber Security Labs B.V..
/// 4. Neither the name of Cyber Security Labs B.V. nor the
///    names of its contributors may be used to endorse or promote products
///    derived from this software without specific prior written permission.
/// 
/// THIS SOFTWARE IS PROVIDED BY Cyber Security Labs B.V. ''AS IS'' AND ANY
/// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
/// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL Cyber Security Labs B.V. BE LIABLE FOR ANY
/// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
/// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
/// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
/// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
