/*
	MPEG_PS
	MPEG Program Stream decode pack
	Author:zhuguli
    The Program Stream System Header contains a 14 bytes length
	Partial Program Stream Pack header format as:
	Name | Number of bits | Description 
	sync bytes 32 0x000001BA 
	marker bits 2 01b 
	System clock [32..30] 3 System Clock Reference (SCR) bits 32 to 30 
	marker bit 1 1 Bit always set. 
	System clock [29..15] 15 System clock bits 29 to 15 
	marker bit 1 1 Bit always set. 
	System clock [14..0] 15 System clock bits 14 to 0 
	marker bit 1 1 Bit always set. 
	SCR	extension 9  
	marker bit 1 1 Bit always set. 
	bit rate 22 In units of 50 bytes per second. 
	marker bits 2 11 Bits always set. 
	reserved 5 reserved for future use 
	stuffing length 3  
	stuffing bytes 8*stuffing length  
	system header (optional) 0 or more if system header start code follows: 0x000001BB
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/types.h>

#include "mpeg_ps.h"

/* mpeg-2 program stream decode for h264.
   ps pack header.
   @t_scr: system clock reference base
   @mux_rate: multiplex rate
   @ps_pack_head: ps_pack_head struct
*/
void PS_Pack_Header(__u64 t_scr,__u32 mux_rate,ps_pack_header_t *ps_pack_head)
{
	t_scr%=0x0000000200000000ll*300;
	t_scr &= 0x000000003fffffffll;
	unsigned int t_scr_ext = (unsigned int)(t_scr%300); /* t_scr = t_scr/300 */
	unsigned short scr_base_1 = (unsigned short)((t_scr&0x000000003fff8000ll)>>15);
	unsigned short scr_base_2 = (unsigned short)(t_scr&0x0000000000007fffll);
	ps_pack_head->pack_header[0] = 0x00;
	ps_pack_head->pack_header[1] = 0x00;
	ps_pack_head->pack_header[2] = 0x01;
	ps_pack_head->pack_header[3] = 0xBA;
	ps_pack_head->n4_byte = (0x1<<6) + (unsigned char)((t_scr>>30)<<3) + (0x1<<2) + ((unsigned char)(scr_base_1>>13)&0x03);
	ps_pack_head->n5_byte = (unsigned char)((scr_base_1&0x1fe0)>>5);
	ps_pack_head->n6_byte = (unsigned char)(scr_base_1<<3) + (0x1<<2) + ((scr_base_2>>13)&0x03);
	ps_pack_head->n7_byte = (unsigned char)((scr_base_2&0x1fe0)>>5);
	ps_pack_head->n8_byte = (unsigned char)(scr_base_2<<3) + (0x1<<2) + ((t_scr_ext>>7)&0x03);
	ps_pack_head->n9_byte = (unsigned char)(t_scr_ext<<1) + 0x01;
	ps_pack_head->n10_byte = (unsigned char)(mux_rate>>14);
	ps_pack_head->n11_byte = (unsigned char)(mux_rate>>6);
	ps_pack_head->n12_byte = (unsigned char)(mux_rate<<2) + 0x03;
	ps_pack_head->n13_byte = (0x1f<<3) + 0x00;
}

/* MPEG Program Stream system header */
void PS_System_Header(ps_system_header_t *ps_system_head)
{
	ps_system_head->sys_header[0] = 0x00;
	ps_system_head->sys_header[1] = 0x00;
	ps_system_head->sys_header[2] = 0x01;
	ps_system_head->sys_header[3] = 0xbb;
	ps_system_head->header_leng[0] = (0x0c&0xFF00)>>8;
	ps_system_head->header_leng[1] = 0x0c&0x00FF; /* 12 bytes */
	ps_system_head->n6_byte = (0x01<<7) + (unsigned char)(0x28F5C>>15);
	ps_system_head->n7_byte = (unsigned char)(0x28F5C>>7); /*  0x28F5C = 167772,85 1E B9 = 101000111101011100 */
	ps_system_head->n8_byte = (unsigned char)(0x28F5C<<1) + 0x01;
	
	ps_system_head->n9_byte = 0x01<<2;
	
	ps_system_head->n10_byte = (0x01<<5) + 0x01;
	ps_system_head->n11_byte = 0x7f;
	
}
/* Program stream system header stream id */
void PS_System_Header_Stream(unsigned char stream_id,
							 ps_system_header_stream_t *ps_system_header_stream
							 )
{
	if(stream_id == 0xe0){
		ps_system_header_stream->stream_id = 0xe0;				            /* stream id */
		ps_system_header_stream->n13_byte = (0x03<<6) + (0x01<<5) + 0x00;   /* PSTD_buffer_bound_scale , marker bit */

		ps_system_header_stream->n14_byte = 0x01; 	                        /* PSTD_buffer_size_bound */
	}
	if(stream_id == 0xc0){
		ps_system_header_stream->stream_id = 0xc0;				/* stream id */
		ps_system_header_stream->n13_byte = (0x03<<6) + 0x00;   /* PSTD_buffer_bound_scale , marker bit */
		ps_system_header_stream->n14_byte = 0x48; 	            /* PSTD_buffer_size_bound */
	}
}
/* Program stream map header */
void PS_Map_Header(	unsigned short map_len,
					unsigned short elem_map_len,
					ps_map_header_t *ps_map_head
				   )
{
	ps_map_head->map_header[0] = 0x00;
	ps_map_head->map_header[1] = 0x00;
	ps_map_head->map_header[2] = 0x01;
	ps_map_head->map_header[3] = 0xBC;
	ps_map_head->map_leng[0] = map_len>>8; /* map leng  = 18 */
	ps_map_head->map_leng[1] = map_len;
	
	ps_map_head->n7_byte = 0xe0;
	
	ps_map_head->n8_byte = 0xff;
	
	ps_map_head->info_len[0] = 0;
	ps_map_head->info_len[1] = 0;
	
	ps_map_head->elem_map_len[0] = elem_map_len>>8;
	ps_map_head->elem_map_len[1] = elem_map_len;
	
}
/* Program stream map header stream struct */
void PS_Map_Header_Stream(unsigned char 	stream_id, /* stream id */
						  unsigned short 	info_len, /* elementary stream info length */
						  ps_map_header_stream_t *ps_map_header_stream /* map header stream struct data */
						  )
{	
	if(stream_id == 0xe0){
		ps_map_header_stream->stream_type = 0x1b;
		ps_map_header_stream->elem_stream_id = stream_id;
		ps_map_header_stream->elem_stream_info_len[0] = info_len>>8;
		ps_map_header_stream->elem_stream_info_len[1] = info_len;

	}
	if(stream_id == 0xc0){
		ps_map_header_stream->stream_type = 0x90;
		ps_map_header_stream->elem_stream_id = stream_id;
		ps_map_header_stream->elem_stream_info_len[0] = info_len>>8;
		ps_map_header_stream->elem_stream_info_len[1] = info_len;
	}
	
}
/* Program stream pes header */
void PS_Pes_Header(unsigned char stream_id, 	/* stream id */
				   unsigned int  pes_pack_len, 	/* pes packet length */
				   unsigned char pes_head_len, 	/* pes header length */
				   unsigned char pts_dts_flag, 	/* pts dts flags */
				   __u64 		 t_pts,		   	/*  pts */
				   __u64		 t_dts,			/*  dts */
				   ps_pes_header_t *ps_pes_head  /*  pes header struct */
				   )
{
	t_pts%=0x0000000200000000ll*300;
	t_pts &= 0x000000003fffffffll;
	t_dts = t_pts;
	ps_pes_head->pes_header[0] = 0x00;
	ps_pes_head->pes_header[1] = 0x00;
	ps_pes_head->pes_header[2] = 0x01;
	ps_pes_head->pes_stream_id = stream_id;
	ps_pes_head->pack_len[0] = pes_pack_len>>8;
	ps_pes_head->pack_len[1] = pes_pack_len;
	
	ps_pes_head->flags = 0x8c;
	
	ps_pes_head->pes_flags = pts_dts_flag<<6;
	
	ps_pes_head->pes_header_len = pes_head_len;

	if(pts_dts_flag == 3){
        ps_pes_head->marker_1 = 0x01 + (((unsigned char)((t_pts>>30)<<1))&0x0e) + 0x30;
	}else{
        ps_pes_head->marker_1 = 0x01 + (((unsigned char)((t_pts>>30)<<1))&0x0e) + 0x20;
	}
	
	ps_pes_head->pts_29_15 = (unsigned char)(((t_pts&0x000000003fff8000ll)>>15)>>7);
	
	ps_pes_head->marker_2 =  (unsigned char)(((t_pts&0x000000003fff8000ll)>>15)<<1) + 0x01;
	
	ps_pes_head->pts_14_0 = (unsigned char)((t_pts&0x0000000000007fffll)>>7);
	ps_pes_head->marker_3 = (unsigned char)((t_pts&0x0000000000007fffll)<<1) + 0x01;

	ps_pes_head->marker_4 = 0x01 + (((unsigned char)((t_dts>>30)<<1))&0x0e) + 0x10;
	
	ps_pes_head->dts_29_15 = (unsigned char)(((t_dts&0x000000003fff8000ll)>>15)>>7);
	
	if(stream_id == 0xe0){
    	ps_pes_head->marker_5 = 0x01 + (unsigned char)(((t_dts&0x000000003fff8000ll)>>15)<<1);
    	
    	ps_pes_head->dts_14_0 = (unsigned char)((t_dts&0x0000000000007fffll)>>7);
    	ps_pes_head->marker_6 = 0x01 + (unsigned char)((t_dts&0x0000000000007fffll)<<1);
	}

}


