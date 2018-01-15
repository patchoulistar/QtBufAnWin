#ifndef _BUFR_
#define _BUFR_

/*#define TEMP_VALUES*/

#define SUCCESS 0
#define NEGATIVE_CODE -1
#define END_NOT_FOUND -2
#define CODE_TOO_BIG -3
#define OPEDES_CANT_PARSE -4
#define IGNORE -5
#define SEGMENT_FAULT -6
#define OPEN_FILE_FAIL -7
#define STAT_FILE_FAIL -8
#define EMPTY_FILE -9
#define ALLOCATE_MEMORY_FAIL -10
#define NULL_POINTER -11
#define NOT_NUMERIC -12
#define OUT_OF_ARRAY_INDEX -13
#define SECLEN_LE0 -14
#define LOAD_TABLEB_FAIL -15
#define LOAD_TABLEC_FAIL -16
#define LOAD_TABLED_FAIL -17
#define SETDES_NOT_FOUND -18
#define ELEDES_NOT_FOUND -19
#define DELAY_REP_FACTOR_TOO_BIG -20
#define SUBSET_NUM_TOO_MANY -21
#define SUBSET_NUM_LE0 -22
#define VALUES_TOO_MANY -23
#define WRONG_DESCRIPTOR -24
#define DES_NUM_LE0 -25
#define DES_NUM_TOO_MANY -26
#define BUFR_NOT_FOUND -27
#define MSGLEN_TOO_SHORT -28
#define NOT_OBSERVE_DATA -29
#define FAIL -30

#define MAX_UNIT_LEN 256
#define EMEANLENGTH 512/*english description length*/
#define CMEANLENGTH 1024/*chinese description length*/
#define DFLENGTH 512 /*directory and filename length*/

#define MISSING_ENC 2.7E27

typedef struct descriptor_item
{
  char code[7];
  int power;
  int  base;
  char unit[256];
  int width;
  int basewidth;
  int infrowidth;
  int delaywidth;
  int newwidth;
  char describe[CMEANLENGTH+1];
  struct descriptor_item *next;
}DescriptorItem;


typedef struct head_info
{
  int headlen;
  int msglen;
  int format;
  int seqnum;
  unsigned char ttaaii[7];
  unsigned char cccc[5];
  unsigned char yygg[7];
  unsigned char bbb[4];
}HeadInfo;

typedef struct section3_info
{
   int length;
   int subsetCount;
   int observe;
   int compress;
   int descriptorCount;
   int codeCount;
   char *originDescriptors;
   DescriptorItem* pdescriptorHead;
   DescriptorItem* pdescriptorTail;
}Section3Info;
typedef struct section4_info
{
	int length;
	int valuesCount;
	int cvalsCount;
	int *subsetValsCount;
	double *values;
	char **cvals;
	int kdlen;
	#ifdef TEMP_VALUES
	double *tmpvalues;
	#endif
}Section4Info;

typedef struct bufr_message
{
   HeadInfo *phead;
   int ksec0[3];
   int ksec1[18];
   Section3Info sec3;
   Section4Info sec4;
}BufrMessage;

typedef int (*ProcessSec4)(double*,int,char *,int,DescriptorItem*,int,void *);

int bufr_init(const char *);
void bufr_end();
int bufrdec(unsigned char *bufrContent,int bufrlen,BufrMessage *bufrmsg);
int bufrdec_file(const char *bufrFileName,BufrMessage *bufrmsg);
void free_bufrmsg(BufrMessage *pbufrmsg);
void print_bufrmsg(BufrMessage *bufrmsg, char *outputFileName);
int process_bufrsec4(Section4Info *psec4,Section3Info *psec3
	,ProcessSec4 process,void *pother);

int bufrdecs(unsigned char *bufrContent,int bufrlen,BufrMessage **pbufrmsgs,int **perrs);
int bufrdecs_file(const char *bufrFileName,BufrMessage **pbufrmsgs,int **perrs);
void free_bufrmsgs(BufrMessage *bufrmsgs,int *errs,int n);
void print_bufrmsgs(BufrMessage *bufrmsgs,int n,char *outputFileName);

int bufrenc(BufrMessage *pbufrmsg,unsigned char **pbufr,int *pbufrlen);
int bufrenc_file(BufrMessage *pbufrmsg,const char *outputFileName);

void print_head_info(HeadInfo *pheadinfo,FILE *ofp);
void  print_bufrsec0(const int *ksec0,FILE *ofp);
void print_bufrsec1(const int *ksec1,FILE *ofp);
void print_bufrsec3(const Section3Info *psec3,FILE *ofp);


#endif
