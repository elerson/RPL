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
#include <inttypes.h>
#include "utils.h"

//Mesage Type
#define ICMPv6 58
#define RPL_TYPE 0x9B // 155
#define FRAG_TYPE 44 // 155
#define ROUT_TYPE 43 // 155
#define DATA_TYPE 60
#define PINGREQUEST_TYPE 8
#define PINGREPLY_TYPE 0

//RPL Message Type
#define DOGAG_DIS_CODE 0x00
#define DOGAG_DIO_CODE 0x01
#define DOGAG_DAO_CODE 0x02
#define DOGAG_DAO_ACK_CODE 0x03
#define DOGAG_DCC_CODE 0x8A

// RPL Message Options
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
 /								in6_addr
 / ******************************************************************/

typedef struct in6_addr_
{
	char addr[2];
}in6_addr;

/*****************************************************************
/								ipv6
******************************************************************/
typedef struct ip6_hdr_
 {
	char ip6_src[2];      /* source address */
	char ip6_dst[2];      /* destination address */
 }ip6_hdr;


typedef struct LowPan0_{
	char IPHC[2];
}LowPan0;

typedef struct LowPan1_{
	char IPHC[2];
	uint8_t next;
}LowPan1;

#define LOWPAN2SIZE 7
typedef struct LowPan2_{
	char IPHC[2];
	ip6_hdr addr;
	uint8_t next;
}LowPan2;

/*****************************************************************
/								icmp6 Headers
****************************************************************/

typedef struct icmp6_hdr_
 {
    uint8_t icmp6_type;   /* type field */
    uint8_t icmp6_code;   /* code field */
 }icmp6_hdr;

//icmp6
struct icmp6{
    uint8_t type;
    uint8_t code;
    char checksum[2];
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

/*****************************************************************
/								icmp6 RPL
****************************************************************/

typedef struct ROUTEINFO_{
	uint8_t  type;
	uint8_t  len;
    uint8_t prefixlength;
    uint8_t resvdprfresvd;
    char lifetime[2];
    uint8_t* prefix;
}ROUTEINFO;

typedef struct DAGCONF_{
	uint8_t  type;
	uint8_t  len;
    uint8_t flagsAPCS;
    uint8_t diointdouble;

    uint8_t diointmin;
    uint8_t dioredun;
    char maxrankincriease[2];

    char minhopincrease[2];
    char ocp[2];

    uint8_t reserverd;
    uint8_t deflifetime;
    char lifeunit[2];
}DAGCONF;

typedef struct RPLTARGET_{
	uint8_t  type;
	uint8_t  len;
    uint8_t flags;
    uint8_t prefixlength;
    uint8_t prefix[2];
}RPLTARGET;

typedef struct TRANSINFO_
{
	uint8_t  type;
	uint8_t  len;
    uint8_t eflags;
    uint8_t pathcontrol;

    uint8_t pathsequence;
    uint8_t pathlifetime;

    uint8_t _parentaddress[2];

}TRANSINFO;

typedef struct SOLINFO_
{
	uint8_t  type;
	uint8_t  len;
    uint8_t rplintance;
    uint8_t vidflag;

    union
    {
		uint8_t _data8[16];
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
    } _prefix;


}PREFIXINFO;

typedef struct TARGETDESC_
{
	uint8_t  type;
	uint8_t  len;
    uint8_t *descriptor;
}TARGETDESC;

typedef struct DIS_{
    uint8_t type;
    uint8_t code;
    char checksum[2];
    uint8_t flags;
    uint8_t reserved;
 }DIS;

typedef struct DIO_{
    uint8_t type;
    uint8_t code;
    char checksum[2];
    uint8_t rplinstanceid;
    uint8_t versionNumber;
    char rank[2];
    uint8_t GO_MOP_Prf;
    uint8_t DTSN;
    uint8_t flags;
    uint8_t reserved;
    char dodagid[2];
 }DIO;

typedef struct DAO_{

    uint8_t type;
    uint8_t code;
    char checksum[2];
    uint8_t rplinstanceid;
    uint8_t kdflags;
    uint8_t reserved;
    uint8_t dao_sequence;
    char dodagid[2];

}DAO;

typedef struct DAO_ACK_{

     uint8_t type;
     uint8_t code;
     char checksum[2];
     uint8_t rplinstanceid;
     uint8_t dreserved;
     uint8_t daosequence;
     uint8_t status;
     char dodagid[2];
}DAO_ACK;

typedef struct DCC_{
    uint8_t type;
    uint8_t code;
    char checksum[2];
    uint8_t rplinstanceid;
    uint8_t rflags;
    char ccnonce[2];
    uint8_t dodagid[2];
    uint32_t destinationcounter;

}DCC;

/*****************************************************************
/								Othes icmp6 messages
/ ****************************************************************/

typedef struct FRAG_{
    uint8_t type;
    uint8_t next;
    uint8_t reserved;
    uint8_t fragoffset[2];
    uint8_t id[2];
}FRAG;

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
typedef struct ECHOREQUEST_{
    uint8_t type;
    uint8_t code;
    uint8_t checksum[2];
    uint8_t identifier[2];
    uint8_t sequence[2];
}ECHOREQUEST;

#define ECHOREPLYSIZE 8
typedef struct ECHOREPLY_{
    uint8_t type;
    uint8_t code;
    uint8_t checksum[2];
    uint8_t identifier[2];
    uint8_t sequence[2];
}ECHOREPLY;

/*****************************************************************
/							Auxiliary functions
****************************************************************/
/**
 * This function calculates the checksum for DIO messages
 * @param  DIO message
 * @return A 16bit number representing the checksum
 */
uint16_t calculateDIOChecksum(DIO* dio);
/**
 * This function calculates the checksum for DAO messages
 * @param  DAO message
 * @return A 16bit number representing the checksum
 */
uint16_t calculateDAOChecksum(DAO* dio);
/**
 * This function calculates the checksum for DAOACK messages
 * @param  DAOACK message
 * @return A 16bit number representing the checksum
 */
uint16_t calculateDAOACKChecksum(DAO_ACK* dio);

/* This function compares the checksum represented in 16bits with a checksum represented in 2 bytes
* @param  Checksum represented in 16bits
* @param  Checksum represented in two number of 8bits
* @return True if the checksums represent the same number. False otherwise.
*/
BOOL compareChecksum(uint16_t checksum1, char checksum2[2]);

#endif
