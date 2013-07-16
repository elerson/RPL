/* Copyright 2013 - Elerson Rubens da S. Santos <elerss@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IP6_H_
#define IP6_H_

#include <stdint.h>

#define ICMPv6 58
#define RPL_TYPE 155
#define UINT16 char[2];

#define RPL_OPTION_PAD1                  0
#define RPL_OPTION_PADN                  1
#define RPL_OPTION_DAG_METRIC_CONTAINER  2
#define RPL_OPTION_ROUTE_INFO            3
#define RPL_OPTION_DAG_CONF              4
#define RPL_OPTION_TARGET                5
#define RPL_OPTION_TRANSIT               6
#define RPL_OPTION_SOLICITED_INFO        7
#define RPL_OPTION_PREFIX_INFO           8
#define RPL_OPTION_TARGET_DESC           9


  /*****************************************************************
  /*								in6_addr
  /******************************************************************/

typedef uint16_t in6_addr;
/*{
	uint16_t addr;
}in6_addr;*/

/*****************************************************************
/*								ipv6
/******************************************************************/
typedef struct ip6_hdr_
 {
    uint16_t ip6_src;      /* source address */
    uint16_t ip6_dst;      /* destination address */
 }ip6_hdr;


typedef struct LowPan0_{
	uint16_t IPHC;
}LowPan0;

#define LowPan1size 3
typedef struct LowPan1_{
	uint16_t IPHC;
	uint8_t next;
}LowPan1;

#define LowPan2size 7
typedef struct LowPan2_{
	uint16_t IPHC;
	ip6_hdr addr;
	uint8_t next;
}LowPan2;


  /*****************************************************************
  /*								icmp6
  /******************************************************************/

struct icmp6_hdr
  {
    uint8_t     icmp6_type;   /* type field */
    uint8_t     icmp6_code;   /* code field */
  };


//icmp6
struct icmp6{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
};

//option structs
struct icmpoption{
    uint8_t  type;
    uint8_t  len;
};

typedef struct PAD1_{
	uint8_t  type;

}PAD1;

typedef struct PADN_{
    uint8_t  type;
	uint8_t  len;
    uint8_t* padding;
}PADN;

typedef struct ROUTEINFO_{
	uint8_t  type;
	uint8_t  len;
    uint8_t prefixlength;
    uint8_t resvdprfresvd;
    uint16_t lifetime;
    uint8_t* prefix;
}ROUTEINFO;

typedef struct DAGCONF_{
	uint8_t  type;
	uint8_t  len;
    uint8_t flagsAPCS;
    uint8_t diointdouble;

    uint8_t diointmin;
    uint8_t dioredun;
    uint16_t maxrankincriease;

    uint16_t minhopincrease;
    uint16_t ocp;

    uint8_t reserverd;
    uint8_t deflifetime;
    uint16_t lifeunit;
}DAGCONF;

struct RPLTARGET{
    uint8_t  type;
    uint8_t  len;
    uint8_t flags;
    uint8_t prefixlength;
    uint16_t prefix;
};


struct TRANSINFO
{
    uint8_t  type;
    uint8_t  len;
    uint8_t eflags;
    uint8_t pathcontrol;

    uint8_t pathsequence;
    uint8_t pathlifetime;

    uint16_t parentaddress;

};

typedef struct SOLINFO_
{
	uint8_t  type;
	uint8_t  len;
    uint8_t rplintance;
    uint8_t vidflag;

    union
    {
		uint8_t _data8[16];
		#if defined __USE_MISC || defined __USE_GNU
		uint16_t _data16[8];
		uint32_t _data32[4];
		#endif
    } _didagid;

    uint8_t versionnumber;

}SOLINFO;


typedef struct PREFIXINFO_
{
	uint8_t  type;
	uint8_t  len;
    uint8_t prefixlength;
    uint8_t larreserved;

    uint32_t lifetime;
    uint32_t preferedlifetime;
    uint32_t reserved;


    union
    {
		uint8_t _data8[16];
		#if defined __USE_MISC || defined __USE_GNU
		uint16_t _data16[8];
		uint32_t _data32[4];
		#endif
    } _prefix;


}PREFIXINFO;


typedef struct TARGETDESC_
{
	uint8_t  type;
	uint8_t  len;
  //  nd_opt_hdr header;
    uint8_t *descriptor;
}TARGETDESC;

//RPL ICMPv6
#define RPL_TYPE 0x9B // 155

#define DOGAG_DIS_CODE 0x00
#define DOGAG_DIO_CODE 0x01
#define DOGAG_DAO_CODE 0x02
#define DOGAG_DAO_ACK_CODE 0x03
#define DOGAG_DCC_CODE 0x8A


struct DIS{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint8_t flags;
    uint8_t reserved;

};

struct DIO{

    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint8_t rplinstanceid;
    uint8_t versionNumber;
    uint16_t rank;
    uint8_t GO_MOP_Prf;
    uint8_t DTSN;
    uint8_t flags;
    uint8_t reserved;
    in6_addr dodagid;

    void calculateDIOChecksum();
 };


#define DAOSIZE 10
 struct DAO{

    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint8_t rplinstanceid;
    uint8_t kdflags;
    uint8_t reserved;
    uint8_t dao_sequence;
    in6_addr dodagid;
    void calculateDAOChecksum();

};
#define DAO_ACKSIZE 10
 struct DAO_ACK{

     uint8_t type;
     uint8_t code;
     uint16_t checksum;
     uint8_t rplinstanceid;
     uint8_t dreserved;
     uint8_t daosequence;
     uint8_t status;
     in6_addr dodagid;
     void calculateDAOACKChecksum();
};

struct DCC{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint8_t rplinstanceid;
    uint8_t rflags;
    uint16_t ccnonce;
    union
    {
		uint8_t __u6_addr8[16];
		#if defined __USE_MISC || defined __USE_GNU
		uint16_t __u6_addr16[8];
		uint32_t __u6_addr32[4];
		#endif
    } _dodagid;
    uint32_t destinationcounter;

};


#define FRAG_TYPE 44 // 155
#define ROUT_TYPE 43 // 155
#define DATA_TYPE 60
#define PINGREQUEST_TYPE 8
#define PINGREPLY_TYPE 0


#define FRAGSIZE 7
struct FRAG{
    uint8_t type;
    uint8_t next;
    uint8_t reserved;
    uint16_t fragoffset;
    uint16_t id;
};

#define ROUTINGSIZE 8
typedef struct ROUTING_{
    uint8_t next;
    uint8_t hdrextlen;
    uint8_t routingtype;
    uint8_t segmentsLeft;
    uint8_t cmprICmprE;
    uint8_t padReververd[3];
}ROUTING;

#define ECHOREQUESTSIZE 8
struct ECHOREQUEST{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
};

#define ECHOREPLYSIZE 8
struct ECHOREPLY{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
};

#endif
