#ifndef __MB_TYPE_H
#define __MB_TYPE_H

typedef enum
{
	FALSE,
	TRUE,
}BOOL;

typedef struct{
	unsigned char type;
	unsigned char funcid;
	unsigned short startadd;
	unsigned short regN;
	unsigned char *databuff;
	unsigned char mode;
}mb_pro_t;

#endif
