/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PACK_H
#define CR_PACK_H

#include "cr_compiler.h"
#include "cr_error.h"
#include "cr_protocol.h"
#include "cr_opcodes.h"
#include "cr_endian.h"
#include "state/cr_statetypes.h"
#include "state/cr_currentpointers.h"
#include "state/cr_client.h"
#ifdef CHROMIUM_THREADSAFE
#include "cr_threads.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CRPackContext_t CRPackContext;

/**
 * Packer buffer
 */
typedef struct
{
	void          *pack;  /**< the actual storage space/buffer */
	unsigned int   size;  /**< size of pack[] buffer */
	unsigned int   mtu;
	unsigned char *data_start, *data_current, *data_end;
	unsigned char *opcode_start, *opcode_current, *opcode_end;
	GLboolean geometry_only;  /**< just used for debugging */
	GLboolean holds_BeginEnd;
	GLboolean in_BeginEnd;
	GLboolean canBarf;
	GLboolean holds_List;
	GLboolean in_List;
	CRPackContext *context;
} CRPackBuffer;

typedef void (*CRPackFlushFunc)(void *arg);
typedef void (*CRPackSendHugeFunc)(CROpcode, void *);
typedef void (*CRPackErrorHandlerFunc)(int line, const char *file, GLenum error, const char *info);


/**
 * Packer context
 */
struct CRPackContext_t
{
	CRPackBuffer buffer;   /**< not a pointer, see comments in pack_buffer.c */
	CRPackFlushFunc Flush;
	void *flush_arg;
	CRPackSendHugeFunc SendHuge;
	CRPackErrorHandlerFunc Error;
	CRCurrentStatePointers current;
	GLvectorf bounds_min, bounds_max;
	int updateBBOX;
	int swapping;
	CRPackBuffer *currentBuffer;
	char *file;  /**< for debugging only */
	int line;    /**< for debugging only */
};


CRPackContext *crPackNewContext(int swapping);
void crPackSetContext( CRPackContext *pc );
CRPackContext *crPackGetContext( void );

void crPackSetBuffer( CRPackContext *pc, CRPackBuffer *buffer );
void crPackSetBufferDEBUG( const char *file, int line, CRPackContext *pc, CRPackBuffer *buffer );
void crPackReleaseBuffer( CRPackContext *pc );
void crPackResetPointers( CRPackContext *pc );

int crPackMaxOpcodes( int buffer_size );
int crPackMaxData( int buffer_size );
void crPackInitBuffer( CRPackBuffer *buffer, void *buf, int size, int mtu );
void crPackFlushFunc( CRPackContext *pc, CRPackFlushFunc ff );
void crPackFlushArg( CRPackContext *pc, void *flush_arg );
void crPackSendHugeFunc( CRPackContext *pc, CRPackSendHugeFunc shf );
void crPackErrorFunction( CRPackContext *pc, CRPackErrorHandlerFunc errf );
void crPackOffsetCurrentPointers( int offset );
void crPackNullCurrentPointers( void );

void crPackResetBoundingBox( CRPackContext *pc );
GLboolean crPackGetBoundingBox( CRPackContext *pc,
                                GLfloat *xmin, GLfloat *ymin, GLfloat *zmin,
                                GLfloat *xmax, GLfloat *ymax, GLfloat *zmax);

void crPackAppendBuffer( const CRPackBuffer *buffer );
void crPackAppendBoundedBuffer( const CRPackBuffer *buffer, const CRrecti *bounds );
int crPackCanHoldBuffer( const CRPackBuffer *buffer );
int crPackCanHoldBoundedBuffer( const CRPackBuffer *buffer );

#if defined(LINUX)
#define CR_UNALIGNED_ACCESS_OKAY
#else
#undef CR_UNALIGNED_ACCESS_OKAY
#endif
void crWriteUnalignedDouble( void *buffer, double d );
void crWriteSwappedDouble( void *buffer, double d );

void *crPackAlloc( unsigned int len );
void crHugePacket( CROpcode op, void *ptr );
void crPackFree( void *ptr );
void crNetworkPointerWrite( CRNetworkPointer *, void * );

void crPackExpandDrawArrays(GLenum mode, GLint first, GLsizei count, CRClientState *c);
void crPackExpandDrawArraysSWAP(GLenum mode, GLint first, GLsizei count, CRClientState *c);

void crPackUnrollDrawElements(GLsizei count, GLenum type, const GLvoid *indices);
void crPackUnrollDrawElementsSWAP(GLsizei count, GLenum type, const GLvoid *indices);

void crPackExpandDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, CRClientState *c);
void crPackExpandDrawElementsSWAP(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, CRClientState *c);

void crPackExpandDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, CRClientState *c);
void crPackExpandDrawRangeElementsSWAP(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, CRClientState *c);

void crPackExpandArrayElement(GLint index, CRClientState *c);
void crPackExpandArrayElementSWAP(GLint index, CRClientState *c);

void crPackExpandMultiDrawArraysEXT( GLenum mode, GLint *first, GLsizei *count, GLsizei primcount, CRClientState *c );
void crPackExpandMultiDrawArraysEXTSWAP( GLenum mode, GLint *first, GLsizei *count, GLsizei primcount, CRClientState *c );

void crPackExpandMultiDrawElementsEXT( GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount, CRClientState *c );
void crPackExpandMultiDrawElementsEXTSWAP( GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount, CRClientState *c );


#ifdef CHROMIUM_THREADSAFE
extern CRtsd _PackerTSD;
#define GET_PACKER_CONTEXT(C) CRPackContext *C = (CRPackContext *) crGetTSD(&_PackerTSD)
#else
extern CRPackContext cr_packer_globals;
#define GET_PACKER_CONTEXT(C) CRPackContext *C = &cr_packer_globals
#endif


/**
 * Return number of opcodes in given buffer.
 */
static INLINE int
crPackNumOpcodes(const CRPackBuffer *buffer)
{
	CRASSERT(buffer->opcode_start - buffer->opcode_current >= 0);
	return buffer->opcode_start - buffer->opcode_current;
}


/**
 * Return amount of data (in bytes) in buffer.
 */
static INLINE int
crPackNumData(const CRPackBuffer *buffer)
{
	CRASSERT(buffer->data_current - buffer->data_start >= 0);
	return buffer->data_current - buffer->data_start; /* in bytes */
}


static INLINE int
crPackCanHoldOpcode(const CRPackContext *pc, int num_opcode, int num_data)
{
  int fitsInMTU, opcodesFit, dataFits;

  CRASSERT(pc->currentBuffer);

  fitsInMTU = (((pc->buffer.data_current - pc->buffer.opcode_current - 1
                 + num_opcode + num_data
                 + 0x3 ) & ~0x3) + sizeof(CRMessageOpcodes)
               <= pc->buffer.mtu);
  opcodesFit = (pc->buffer.opcode_current - num_opcode >= pc->buffer.opcode_end);
  dataFits = (pc->buffer.data_current + num_data <= pc->buffer.data_end);

  return fitsInMTU && opcodesFit && dataFits;
}


/**
 * Alloc space for a message of 'len' bytes (plus 1 opcode).
 * Only flush if buffer is full.
 */
#define GET_BUFFERED_POINTER_NO_BEGINEND_FLUSH( pc, len )	\
  do {								\
    THREADASSERT( pc );						\
    CRASSERT( pc->currentBuffer );				\
    if ( !crPackCanHoldOpcode( pc, 1, (len) ) ) {		\
      pc->Flush( pc->flush_arg );				\
      CRASSERT(crPackCanHoldOpcode( pc, 1, (len) ) );		\
    }								\
    data_ptr = pc->buffer.data_current;				\
    pc->buffer.data_current += (len);				\
  } while (0)


/**
 * As above, flush if the buffer contains vertex data and we're
 * no longer inside glBegin/glEnd.
 */
#define GET_BUFFERED_POINTER( pc, len )					\
  do {									\
    CRASSERT( pc->currentBuffer );					\
    if ( pc->buffer.holds_BeginEnd && !pc->buffer.in_BeginEnd ) {	\
      pc->Flush( pc->flush_arg );					\
      pc->buffer.holds_BeginEnd = 0;					\
    }									\
    GET_BUFFERED_POINTER_NO_BEGINEND_FLUSH( pc, len );			\
  } while (0)


/**
 * As above, but for vertex data between glBegin/End (counts vertices).
 */
#define GET_BUFFERED_COUNT_POINTER( pc, len )		\
  do {							\
    CRASSERT( pc->currentBuffer );			\
    if ( !crPackCanHoldOpcode( pc, 1, (len) ) ) {	\
      pc->Flush( pc->flush_arg );			\
      CRASSERT( crPackCanHoldOpcode( pc, 1, (len) ) );	\
    }							\
    data_ptr = pc->buffer.data_current;			\
    pc->current.vtx_count++;				\
    pc->buffer.data_current += (len);			\
  } while (0)


/**
 * Allocate space for a msg/command that has no arguments, such
 * as glFinish().
 */
#define GET_BUFFERED_POINTER_NO_ARGS( pc ) \
  GET_BUFFERED_POINTER( pc, 4 );  \
  WRITE_DATA( 0, GLuint, 0xdeadbeef )

#define WRITE_DATA( offset, type, data ) \
  *( (type *) (data_ptr + (offset))) = (data)

#ifdef CR_UNALIGNED_ACCESS_OKAY
#define WRITE_DOUBLE( offset, data ) \
  WRITE_DATA( offset, GLdouble, data )
#else
#define WRITE_DOUBLE( offset, data ) \
  crWriteUnalignedDouble( data_ptr + (offset), (data) )
#endif

#define WRITE_SWAPPED_DOUBLE( offset, data ) \
	crWriteSwappedDouble( data_ptr + (offset), (data) )

#define WRITE_OPCODE( pc, opcode )  \
  *(pc->buffer.opcode_current--) = (unsigned char) opcode

#define WRITE_NETWORK_POINTER( offset, data ) \
  crNetworkPointerWrite( (CRNetworkPointer *) ( data_ptr + (offset) ), (data) )

#ifdef __cplusplus
}
#endif

#endif /* CR_PACK_H */
