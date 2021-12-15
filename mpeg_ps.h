/*
	MPEG-2 PS Header 
	author:zhuguli
*/
#ifndef __MPEG_PS__H
#define __MPEG_PS__H

#include <stdio.h>
#include <unistd.h>
#include <linux/types.h>

#define PS_CODE_PACK_HDR 0x000001BA
#define PS_CODE_SYS_HDR	 0x000001BB
#define PS_CODE_MAP_HDR	 0x000001BC
#define PS_CODE_PES_HDR	 0x000001BD

typedef struct
{
	unsigned char 	pack_header[4];		/* pack header bytes(0x000001BA) */
	unsigned char 	n4_byte;	/* marker bits '01b',system clock refernce base:3bits,marker:1,scr base:2bits */
	unsigned char 	n5_byte;	/* System clock [27..20] */
	unsigned char  	n6_byte;	/* System clock [19..15],marker bit,SCR:2bits */
	unsigned char	n7_byte; 	/* System clock [12..5] */
	unsigned char 	n8_byte;	/* System clock [4..0], marker bit,SCR EXT:2 */
	unsigned char	n9_byte;	/* SCR EXT:7bit,marker bit  */
	unsigned char 	n10_byte;	/* multiplex rate */
	unsigned char  	n11_byte;	/*  */
	unsigned char	n12_byte;	/* marker bit */
	unsigned char 	n13_byte;	/* bit rate(n units of 50 bytes per second.) */
}ps_pack_header_t;/* mpeg-2 program stream pack header */

typedef struct 
{
	unsigned char 	stream_id;		/* stream id */
	unsigned char 	n13_byte;		/* marker bit:2bit ,PSTD_buffer_bound_scale:1bit,PSTD_buffer_size_bound:6bit*/
	unsigned char   n14_byte; 		/*  PSTD_buffer_size_bound*/
}ps_system_header_stream_t;

/* The Program Stream System Header contains a 9 bytes fixed portion followed
	by any number of 3-byte stream_bound entries. */
typedef struct 
{
	unsigned char 	sys_header[4]; 	/* start code */
	unsigned char 	header_leng[2]; /* header length */
	unsigned char	n6_byte;	/* marker bit:1bit, rate bound:7*/
	unsigned char 	n7_byte; 	/* rate bound */
	unsigned char	n8_byte;	/* rate bound,marker bit */
	unsigned char   n9_byte;	/* audio bound, fixed flag:1 ,csps flag:1*/
	unsigned char 	n10_byte;	/* system audio lock flag:1,system video lock glag, marker bit,video bount*/
	unsigned char	n11_byte;	/* restriction flag ,reserver*/
	
}ps_system_header_t; 

typedef struct
{
	unsigned char 	stream_type;	/* stream type */
	unsigned char 	elem_stream_id;	/* elementary_stream_id */
	unsigned char   elem_stream_info_len[2]; /* elementary stream info length is 6 */
}ps_map_header_stream_t;


typedef struct
{
	/* crc (2e b9 0f 3d) */
	unsigned char	crc4; /* crc (24~31) bits */
	unsigned char	crc3; /* crc (16~23) bits */
	unsigned char	crc2; /* crc (8~15) bits */
	unsigned char	crc1; /* crc (0~7) bits */

}ps_map_header_crc_t;

typedef struct
{
	unsigned char 	map_header[4]; 	/* ps map header */
	unsigned char	map_leng[2];	/* program stream map length  */
	
	unsigned char	n7_byte;		/* */
	
	unsigned char 	n8_byte;		/* */
	
	unsigned char	info_len[2];		/* programe stream info length */
	unsigned char	elem_map_len[2];	/* elementary stream map length  is 8*/

	/* ps_map_header_stream_t */

}ps_map_header_t; 		/* The Program Stream map contains a 20 bytes */

/* The Program Stream elementary stream contains a 19 bytes */
typedef struct 
{
	unsigned char	pes_header[3];	/* start code */
	unsigned char	pes_stream_id; 	/* stream id */
	unsigned char	pack_len[2];		/* pes packet leng */
	unsigned char	flags;			/* flags */
	
	unsigned char	pes_flags;		/* flags, dts pts flag */
			
	
	unsigned char	pes_header_len;	/* pes header data length */
	/* PtsDtsFlag  == 3 */
	unsigned char	marker_1;		/* marker bit, presentation time stamp,0010-->pts,0011-->dts*/
	
	unsigned char   pts_29_15;		/* pts bits 29...15 */
	unsigned char	marker_2;		/* pts bits 29...15,marker bit */
	
	unsigned char	pts_14_0;		/* pts bits 14...0 */
	unsigned char	marker_3;		/* pts bits 14...0 ,marker bit, */
	
	
	unsigned char	marker_4;		/* marker bit, decoding time stamp,0001*/
	
	unsigned char	dts_29_15;		/* dts bits 29..15 */
	unsigned char	marker_5;		/* dts bits 29..15,marker bit */
	
	unsigned char	dts_14_0;		/* dts bits 14..0 */
	unsigned char   marker_6;		/* dts bits 14..0,marker bit */
	
	
}ps_pes_header_t;

/* mpeg ps lib function */

/* mpeg-2 program stream decode for h264.
   ps pack header.
   t_scr: system clock reference base
   mux_rate: multiplex rate
   ps_pack_head: ps_pack_head struct
*/
void PS_Pack_Header(__u64 t_scr,__u32 mux_rate,ps_pack_header_t *ps_pack_head);
/* MPEG Program Stream system header */
void PS_System_Header(ps_system_header_t *ps_system_head);
/* Program stream system header stream id */
void PS_System_Header_Stream(unsigned char stream_id,
							 ps_system_header_stream_t *ps_system_header_stream
							 );
/* Program stream map header */
void PS_Map_Header(	unsigned short map_len,
					unsigned short elem_map_len,
					ps_map_header_t *ps_map_head
				   );
/* Program stream map header stream struct */
void PS_Map_Header_Stream(unsigned char 	stream_id, /* stream id */
						  unsigned short 	info_len, /* elementary stream info length */
						  ps_map_header_stream_t *ps_map_header_stream /* map header stream struct data */
						  );
/* Program stream pes header */
void PS_Pes_Header(unsigned char stream_id, 	/* stream id */
				   unsigned int  pes_pack_len, 	/* pes packet length */
				   unsigned char pes_head_len, 	/* pes header length */
				   unsigned char pts_dts_flag, 	/* pts dts flags */
				   __u64 		 t_pts,		   	/*  pts */
				   __u64		 t_dts,			/*  dts */
				   ps_pes_header_t *ps_pes_head /*  pes header struct */
				   );



#endif
