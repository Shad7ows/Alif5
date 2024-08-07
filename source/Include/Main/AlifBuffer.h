#pragma once


class AlifBuffer {
public:
    void* buf{};
    AlifObject* obj{};
    int64_t len{};
    int64_t itemSize{};
    int readonly{};
    int nDim{};
    wchar_t* format{};
    int64_t* shape{};
    int64_t* strides{};
    int64_t* subOffSets{};
    void* internal{};
};

typedef int (*GetBufferProc)(AlifObject*, AlifBuffer*, int);
typedef void (*ReleaseBufferProc)(AlifObject*, AlifBuffer*);


int alifObject_getBuffer(AlifObject* , AlifBuffer* , int );

int alifBuffer_isContiguous(const AlifBuffer* , wchar_t );

int alifBuffer_fillInfo(AlifBuffer* , AlifObject* , void* , int64_t , 
	int , int );

void alifBuffer_release(AlifBuffer* );

#define ALIFBUF_MAX_NDIM 64

/* Flags for getting buffers. Keep these in sync with inspect.BufferFlags. */
#define ALIFBUF_SIMPLE 0
#define ALIFBUF_WRITABLE 0x0001


#define ALIFBUF_FORMAT 0x0004
#define ALIFBUF_ND 0x0008
#define ALIFBUF_STRIDES (0x0010 | ALIFBUF_ND)
#define ALIFBUF_C_CONTIGUOUS (0x0020 | ALIFBUF_STRIDES)
#define ALIFBUF_F_CONTIGUOUS (0x0040 | ALIFBUF_STRIDES)
#define ALIFBUF_ANY_CONTIGUOUS (0x0080 | ALIFBUF_STRIDES)
#define ALIFBUF_INDIRECT (0x0100 | ALIFBUF_STRIDES)

#define ALIFBUF_CONTIG (ALIFBUF_ND | ALIFBUF_WRITABLE)
#define ALIFBUF_CONTIG_RO (ALIFBUF_ND)

#define ALIFBUF_STRIDED (ALIFBUF_STRIDES | ALIFBUF_WRITABLE)
#define ALIFBUF_STRIDED_RO (ALIFBUF_STRIDES)

#define ALIFBUF_RECORDS (ALIFBUF_STRIDES | ALIFBUF_WRITABLE | ALIFBUF_FORMAT)
#define ALIFBUF_RECORDS_RO (ALIFBUF_STRIDES | ALIFBUF_FORMAT)

#define ALIFBUF_FULL (ALIFBUF_INDIRECT | ALIFBUF_WRITABLE | ALIFBUF_FORMAT)
#define ALIFBUF_FULL_RO (ALIFBUF_INDIRECT | ALIFBUF_FORMAT)


#define ALIFBUF_READ  0x100
#define ALIFBUF_WRITE 0x200
