/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_pixeldata.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_version.h"


/**
 * Maybe export this someday.
 */
static int crSizeOfType( GLenum type )
{
	switch (type) {
#ifdef CR_OPENGL_VERSION_1_2
		case GL_UNSIGNED_BYTE_3_3_2:
		case GL_UNSIGNED_BYTE_2_3_3_REV:
#endif
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			return 1;
		case GL_BITMAP:
			return 0;  /* special case */
#ifdef CR_OPENGL_VERSION_1_2
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_5_6_5_REV:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
#endif
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			return 2;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_UNSIGNED_INT_8_8_8_8:
		case GL_UNSIGNED_INT_8_8_8_8_REV:
		case GL_UNSIGNED_INT_10_10_10_2:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
#endif
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_FLOAT:
			return 4;
		case GL_DOUBLE:
			return 8;
		default: 
			crError( "Unknown pixel type in crSizeOfType: 0x%x", (unsigned int) type );
			return 0;
	}
}


/**
 * Compute bytes per pixel for the given format/type combination.
 * \return bytes per pixel or -1 for invalid format or type, 0 for bitmap data.
 */
int crPixelSize( GLenum format, GLenum type )
{
	int bytes = 1; /* picky Windows compiler, we override later */

	switch (type) {
#ifdef CR_OPENGL_VERSION_1_2
		case GL_UNSIGNED_BYTE_3_3_2:
		case GL_UNSIGNED_BYTE_2_3_3_REV:
			return 1;
#endif
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			bytes = 1;
			break;
		case GL_BITMAP:
			return 0;  /* special case */
#ifdef CR_OPENGL_VERSION_1_2
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_5_6_5_REV:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
			return 2;
#endif
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			bytes = 2;
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_UNSIGNED_INT_8_8_8_8:
		case GL_UNSIGNED_INT_8_8_8_8_REV:
		case GL_UNSIGNED_INT_10_10_10_2:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			return 4;
#endif
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_FLOAT:
			bytes = 4;
			break;
		default: 
			/*
			crError( "Unknown pixel type in crPixelSize: 0x%x", (unsigned int) type );
			*/
			return -1;
	}

	switch (format) {
		case GL_COLOR_INDEX:
		case GL_STENCIL_INDEX:
		case GL_DEPTH_COMPONENT:
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
		case GL_LUMINANCE:
		case GL_INTENSITY:
			break;
		case GL_LUMINANCE_ALPHA:
			bytes *= 2;
			break;
		case GL_RGB:
#ifdef CR_OPENGL_VERSION_1_2
		case GL_BGR:
#endif
			bytes *= 3;
			break;
		case GL_RGBA:
#ifdef GL_ABGR_EXT
		case GL_ABGR_EXT:
#endif
#ifdef CR_OPENGL_VERSION_1_2
		case GL_BGRA:
#endif
			bytes *= 4;
			break;
		default:
			/*
			crError( "Unknown pixel format in crPixelSize: 0x%x", (unsigned int) format );
			*/
			return -1;
	}

	return bytes;
}


#define BYTE_TO_FLOAT(b)  ((b) * (1.0/127.0))
#define FLOAT_TO_BYTE(f)  ((GLbyte) ((f) * 127.0))

#define UBYTE_TO_FLOAT(b)  ((b) * (1.0/255.0))
#define FLOAT_TO_UBYTE(f)  ((GLbyte) ((f) * 255.0))

#define SHORT_TO_FLOAT(s)  ((s) * (1.0/32768.0))
#define FLOAT_TO_SHORT(f)  ((GLshort) ((f) * 32768.0))

#define USHORT_TO_FLOAT(s)  ((s) * (1.0/65535.0))
#define FLOAT_TO_USHORT(f)  ((GLushort) ((f) * 65535.0))

#define INT_TO_FLOAT(i)  ((i) * (1.0F/2147483647.0))
#define FLOAT_TO_INT(f)  ((GLint) ((f) * 2147483647.0))

#define UINT_TO_FLOAT(i)  ((i) * (1.0F / 4294967295.0F))
#define FLOAT_TO_UINT(f)  ((GLuint) ((f) * 4294967295.0))



/*
 * Pack src pixel data into tmpRow array as either GLfloat[][1] or
 * GLfloat[][4] depending on whether the format is for colors.
 */
static void
get_row(const char *src, GLenum srcFormat, GLenum srcType,
		GLsizei width, GLfloat *tmpRow)
{
	const GLbyte *bSrc = (GLbyte *) src;
	const GLubyte *ubSrc = (GLubyte *) src;
	const GLshort *sSrc = (GLshort *) src;
	const GLushort *usSrc = (GLushort *) src;
	const GLint *iSrc = (GLint *) src;
	const GLuint *uiSrc = (GLuint *) src;
	const GLfloat *fSrc = (GLfloat *) src;
	const GLdouble *dSrc = (GLdouble *) src;
	int i;

	if (srcFormat == GL_COLOR_INDEX || srcFormat == GL_STENCIL_INDEX) {
		switch (srcType) {
			case GL_BYTE:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) bSrc[i];
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) ubSrc[i];
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) sSrc[i];
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) usSrc[i];
				break;
			case GL_INT:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) iSrc[i];
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) uiSrc[i];
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++)
					tmpRow[i] = fSrc[i];
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) dSrc[i];
				break;
			default:
				crError("unexpected type in get_row in pixel.c");
		}
	}
	else if (srcFormat == GL_DEPTH_COMPONENT) {
		switch (srcType) {
			case GL_BYTE:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) BYTE_TO_FLOAT(bSrc[i]);
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i]);
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) SHORT_TO_FLOAT(sSrc[i]);
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) USHORT_TO_FLOAT(usSrc[i]);
				break;
			case GL_INT:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) INT_TO_FLOAT(bSrc[i]);
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) UINT_TO_FLOAT(bSrc[i]);
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++)
					tmpRow[i] = fSrc[i];
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++)
					tmpRow[i] = (GLfloat) dSrc[i];
				break;
			default:
				crError("unexpected type in get_row in pixel.c");
		}
	}
	else if (srcFormat == GL_RED || srcFormat == GL_GREEN ||
					 srcFormat == GL_BLUE || srcFormat == GL_ALPHA) {
		int dst;
		if (srcFormat == GL_RED)
			dst = 0;
		else if (srcFormat == GL_GREEN)
			dst = 1;
		else if (srcFormat == GL_BLUE)
			dst = 2;
		else
			dst = 3;
		for (i = 0; i < width; i++) {
			tmpRow[i*4+0] = 0.0;
			tmpRow[i*4+1] = 0.0;
			tmpRow[i*4+2] = 0.0;
			tmpRow[i*4+3] = 1.0;
		}
		switch (srcType) {
			case GL_BYTE:
				for (i = 0; i < width; i++, dst += 4)
					tmpRow[dst] = (GLfloat) BYTE_TO_FLOAT(bSrc[i]);
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++, dst += 4)
					tmpRow[dst] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i]);
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++, dst += 4)
					tmpRow[dst] = (GLfloat) SHORT_TO_FLOAT(sSrc[i]);
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++, dst += 4)
					tmpRow[dst] = (GLfloat) USHORT_TO_FLOAT(usSrc[i]);
				break;
			case GL_INT:
				for (i = 0; i < width; i++, dst += 4)
					tmpRow[dst] = (GLfloat) INT_TO_FLOAT(iSrc[i]);
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++, dst += 4)
					tmpRow[dst] = (GLfloat) UINT_TO_FLOAT(uiSrc[i]);
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++, dst += 4)
					tmpRow[dst] = fSrc[i];
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++, dst += 4)
					tmpRow[dst] = (GLfloat) fSrc[i];
				break;
			default:
				crError("unexpected type in get_row in pixel.c");
		}
	}
	else if (srcFormat == GL_LUMINANCE) {
		switch (srcType) {
			case GL_BYTE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) BYTE_TO_FLOAT(bSrc[i]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) UBYTE_TO_FLOAT(ubSrc[i]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) SHORT_TO_FLOAT(sSrc[i]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) USHORT_TO_FLOAT(usSrc[i]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_INT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) INT_TO_FLOAT(iSrc[i]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) UINT_TO_FLOAT(uiSrc[i]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]	= fSrc[i];
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = (GLfloat) dSrc[i];
					tmpRow[i*4+3] = 1.0;
				}
				break;
			default:
				crError("unexpected type in get_row in pixel.c");
		}
	}
	else if (srcFormat == GL_INTENSITY) {
		switch (srcType) {
			case GL_BYTE:
				for (i = 0; i < width; i++)
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
						= (GLfloat) BYTE_TO_FLOAT(bSrc[i]);
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++)
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
						= (GLfloat) UBYTE_TO_FLOAT(ubSrc[i]);
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++)
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
						= (GLfloat) SHORT_TO_FLOAT(sSrc[i]);
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++)
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
						= (GLfloat) USHORT_TO_FLOAT(usSrc[i]);
				break;
			case GL_INT:
				for (i = 0; i < width; i++)
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
						= (GLfloat) INT_TO_FLOAT(iSrc[i]);
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++)
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
						= UINT_TO_FLOAT(uiSrc[i]);
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++)
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
						= fSrc[i];
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++)
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
						= (GLfloat) dSrc[i];
				break;
			default:
				crError("unexpected type in get_row in pixel.c");
		}
	}
	else if (srcFormat == GL_LUMINANCE_ALPHA) {
		switch (srcType) {
			case GL_BYTE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) BYTE_TO_FLOAT(bSrc[i*2+0]);
					tmpRow[i*4+3] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*2+1]);
				}
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*2+0]);
					tmpRow[i*4+3] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*2+1]);
				}
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) SHORT_TO_FLOAT(sSrc[i*2+0]);
					tmpRow[i*4+3] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*2+1]);
				}
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) USHORT_TO_FLOAT(usSrc[i*2+0]);
					tmpRow[i*4+3] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*2+1]);
				}
				break;
			case GL_INT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) INT_TO_FLOAT(iSrc[i*2+0]);
					tmpRow[i*4+3] = (GLfloat) INT_TO_FLOAT(iSrc[i*2+1]);
				}
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) UINT_TO_FLOAT(uiSrc[i*2+0]);
					tmpRow[i*4+3] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*2+1]);
				}
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]	= fSrc[i*2+0];
					tmpRow[i*4+3] = fSrc[i*2+1];
				}
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
						= (GLfloat) dSrc[i*2+0];
					tmpRow[i*4+3] = (GLfloat) dSrc[i*2+1];
				}
				break;
			default:
				crError("unexpected type in get_row in pixel.c");
		}
	}
	else if (srcFormat == GL_RGB 
#ifdef CR_OPENGL_VERSION_1_2
					 || srcFormat == GL_BGR 
#endif
					 ) {
		int r, b;
		if (srcFormat == GL_RGB) {
			r = 0; b = 2;
		}
		else {
			r = 2; b = 0;
		}
		switch (srcType) {
			case GL_BYTE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*3+r]);
					tmpRow[i*4+1] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*3+1]);
					tmpRow[i*4+2] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*3+b]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*3+r]);
					tmpRow[i*4+1] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*3+1]);
					tmpRow[i*4+2] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*3+b]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*3+r]);
					tmpRow[i*4+1] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*3+1]);
					tmpRow[i*4+2] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*3+b]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*3+r]);
					tmpRow[i*4+1] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*3+1]);
					tmpRow[i*4+2] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*3+b]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_INT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) INT_TO_FLOAT(iSrc[i*3+r]);
					tmpRow[i*4+1] = (GLfloat) INT_TO_FLOAT(iSrc[i*3+1]);
					tmpRow[i*4+2] = (GLfloat) INT_TO_FLOAT(iSrc[i*3+b]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*3+r]);
					tmpRow[i*4+1] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*3+1]);
					tmpRow[i*4+2] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*3+b]);
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = fSrc[i*3+r];
					tmpRow[i*4+1] = fSrc[i*3+1];
					tmpRow[i*4+2] = fSrc[i*3+b];
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) dSrc[i*3+r];
					tmpRow[i*4+1] = (GLfloat) dSrc[i*3+1];
					tmpRow[i*4+2] = (GLfloat) dSrc[i*3+b];
					tmpRow[i*4+3] = 1.0;
				}
				break;
#ifdef CR_OPENGL_VERSION_1_2
			case GL_UNSIGNED_BYTE_3_3_2:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((ubSrc[i] >> 5)      ) / 7.0f;
					tmpRow[i*4+1] = (GLfloat) ((ubSrc[i] >> 2) & 0x7) / 7.0f;
					tmpRow[i*4+b] = (GLfloat) ((ubSrc[i]     ) & 0x3) / 3.0f;
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_UNSIGNED_BYTE_2_3_3_REV:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((ubSrc[i]     ) & 0x7) / 7.0f;
					tmpRow[i*4+1] = (GLfloat) ((ubSrc[i] >> 3) & 0x7) / 7.0f;
					tmpRow[i*4+b] = (GLfloat) ((ubSrc[i] >> 6)      ) / 3.0f;
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_UNSIGNED_SHORT_5_6_5:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((usSrc[i] >> 11) & 0x1f) / 31.0f;
					tmpRow[i*4+1] = (GLfloat) ((usSrc[i] >>  5) & 0x3f) / 63.0f;
					tmpRow[i*4+b] = (GLfloat) ((usSrc[i]      ) & 0x1f) / 31.0f;
					tmpRow[i*4+3] = 1.0;
				}
				break;
			case GL_UNSIGNED_SHORT_5_6_5_REV:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((usSrc[i]      ) & 0x1f) / 31.0f;
					tmpRow[i*4+1] = (GLfloat) ((usSrc[i] >>  5) & 0x3f) / 63.0f;
					tmpRow[i*4+b] = (GLfloat) ((usSrc[i] >> 11) & 0x1f) / 31.0f;
					tmpRow[i*4+3] = 1.0;
				}
				break;
#endif
			default:
				crError("unexpected type in get_row in pixel.c");
		}
	}
	else if (srcFormat == GL_RGBA
#ifdef CR_OPENGL_VERSION_1_2
					 || srcFormat == GL_BGRA 
#endif		
					 ) {
		int r, b;
		if (srcFormat == GL_RGB) {
			r = 0; b = 2;
		}
		else {
			r = 2; b = 0;
		}
		switch (srcType) {
			case GL_BYTE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*4+r]);
					tmpRow[i*4+1] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*4+1]);
					tmpRow[i*4+2] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*4+b]);
					tmpRow[i*4+3] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*4+3]);
				}
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*4+r]);
					tmpRow[i*4+1] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*4+1]);
					tmpRow[i*4+2] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*4+b]);
					tmpRow[i*4+3] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*4+3]);
				}
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*4+r]);
					tmpRow[i*4+1] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*4+1]);
					tmpRow[i*4+2] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*4+b]);
					tmpRow[i*4+3] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*4+3]);
				}
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*4+r]);
					tmpRow[i*4+1] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*4+1]);
					tmpRow[i*4+2] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*4+b]);
					tmpRow[i*4+3] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*4+3]);
				}
				break;
			case GL_INT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) INT_TO_FLOAT(iSrc[i*4+r]);
					tmpRow[i*4+1] = (GLfloat) INT_TO_FLOAT(iSrc[i*4+1]);
					tmpRow[i*4+2] = (GLfloat) INT_TO_FLOAT(iSrc[i*4+b]);
					tmpRow[i*4+3] = (GLfloat) INT_TO_FLOAT(iSrc[i*4+3]);
				}
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*4+r]);
					tmpRow[i*4+1] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*4+1]);
					tmpRow[i*4+2] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*4+b]);
					tmpRow[i*4+3] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*4+3]);
				}
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = fSrc[i*4+r];
					tmpRow[i*4+1] = fSrc[i*4+1];
					tmpRow[i*4+2] = fSrc[i*4+b];
					tmpRow[i*4+3] = fSrc[i*4+3];
				}
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = (GLfloat) dSrc[i*4+r];
					tmpRow[i*4+1] = (GLfloat) dSrc[i*4+1];
					tmpRow[i*4+2] = (GLfloat) dSrc[i*4+b];
					tmpRow[i*4+3] = (GLfloat) dSrc[i*4+3];
				}
				break;
#ifdef CR_OPENGL_VERSION_1_2
			case GL_UNSIGNED_SHORT_5_5_5_1:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((usSrc[i] >> 11)       ) / 31.0f;
					tmpRow[i*4+1] = (GLfloat) ((usSrc[i] >>  6) & 0x1f) / 31.0f;
					tmpRow[i*4+b] = (GLfloat) ((usSrc[i] >>  1) & 0x1f) / 31.0f;
					tmpRow[i*4+3] = (GLfloat) ((usSrc[i]      ) & 0x01);
				}
				break;
			case GL_UNSIGNED_SHORT_1_5_5_5_REV:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((usSrc[i]      ) & 0x1f) / 31.0f;
					tmpRow[i*4+1] = (GLfloat) ((usSrc[i] >>  5) & 0x1f) / 31.0f;
					tmpRow[i*4+b] = (GLfloat) ((usSrc[i] >> 10) & 0x1f) / 31.0f;
					tmpRow[i*4+3] = (GLfloat) ((usSrc[i] >> 15)       );
				}
				break;
			case GL_UNSIGNED_SHORT_4_4_4_4:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((usSrc[i] >> 12) & 0xf) / 15.0f;
					tmpRow[i*4+1] = (GLfloat) ((usSrc[i] >>  8) & 0xf) / 15.0f;
					tmpRow[i*4+b] = (GLfloat) ((usSrc[i] >>  4) & 0xf) / 15.0f;
					tmpRow[i*4+3] = (GLfloat) ((usSrc[i]      ) & 0xf) / 15.0f;
				}
				break;
			case GL_UNSIGNED_SHORT_4_4_4_4_REV:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((usSrc[i]      ) & 0xf) / 15.0f;
					tmpRow[i*4+1] = (GLfloat) ((usSrc[i] >>  4) & 0xf) / 15.0f;
					tmpRow[i*4+b] = (GLfloat) ((usSrc[i] >>  8) & 0xf) / 15.0f;
					tmpRow[i*4+3] = (GLfloat) ((usSrc[i] >> 12) & 0xf) / 15.0f;
				}
				break;
			case GL_UNSIGNED_INT_8_8_8_8:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((uiSrc[i] >> 24) & 0xff) / 255.0f;
					tmpRow[i*4+1] = (GLfloat) ((uiSrc[i] >> 16) & 0xff) / 255.0f;
					tmpRow[i*4+b] = (GLfloat) ((uiSrc[i] >>  8) & 0xff) / 255.0f;
					tmpRow[i*4+3] = (GLfloat) ((uiSrc[i]      ) & 0xff) / 255.0f;
				}
				break;
			case GL_UNSIGNED_INT_8_8_8_8_REV:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((uiSrc[i]      ) & 0xff) / 255.0f;
					tmpRow[i*4+1] = (GLfloat) ((uiSrc[i] >>  8) & 0xff) / 255.0f;
					tmpRow[i*4+b] = (GLfloat) ((uiSrc[i] >> 16) & 0xff) / 255.0f;
					tmpRow[i*4+3] = (GLfloat) ((uiSrc[i] >> 24) & 0xff) / 255.0f;
				}
				break;
			case GL_UNSIGNED_INT_10_10_10_2:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((uiSrc[i] >> 22) & 0x3ff) / 1023.0f;
					tmpRow[i*4+1] = (GLfloat) ((uiSrc[i] >> 12) & 0x3ff) / 1023.0f;
					tmpRow[i*4+b] = (GLfloat) ((uiSrc[i] >>  2) & 0x3ff) / 1023.0f;
					tmpRow[i*4+3] = (GLfloat) ((uiSrc[i]      ) & 0x003) /    3.0f;
				}
				break;
			case GL_UNSIGNED_INT_2_10_10_10_REV:
				for (i = 0; i < width; i++) {
					tmpRow[i*4+r] = (GLfloat) ((uiSrc[i]      ) & 0x3ff) / 1023.0f;
					tmpRow[i*4+1] = (GLfloat) ((uiSrc[i] >> 10) & 0x3ff) / 1023.0f;
					tmpRow[i*4+b] = (GLfloat) ((uiSrc[i] >> 20) & 0x3ff) / 1023.0f;
					tmpRow[i*4+3] = (GLfloat) ((uiSrc[i] >> 30) & 0x003) /    3.0f;
				}
				break;
#endif
			default:
				crError("unexpected type in get_row in pixel.c");
		}
	}
	else{
		crError("unexpected source format in get_row in pixel.c");
	}
}


static void put_row(char *dst, GLenum dstFormat, GLenum dstType,
										GLsizei width, const GLfloat *tmpRow)
{
	GLbyte *bDst = (GLbyte *) dst;
	GLubyte *ubDst = (GLubyte *) dst;
	GLshort *sDst = (GLshort *) dst;
	GLushort *usDst = (GLushort *) dst;
	GLint *iDst = (GLint *) dst;
	GLuint *uiDst = (GLuint *) dst;
	GLfloat *fDst = (GLfloat *) dst;
	GLdouble *dDst = (GLdouble *) dst;
	int i;

	if (dstFormat == GL_COLOR_INDEX || dstFormat == GL_STENCIL_INDEX) {
		switch (dstType) {
			case GL_BYTE:
				for (i = 0; i < width; i++)
					bDst[i] = (GLbyte) tmpRow[i];
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++)
					ubDst[i] = (GLubyte) tmpRow[i];
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++)
					sDst[i] = (GLshort) tmpRow[i];
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++)
					usDst[i] = (GLushort) tmpRow[i];
				break;
			case GL_INT:
				for (i = 0; i < width; i++)
					iDst[i] = (GLint) tmpRow[i];
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++)
					uiDst[i] = (GLuint) tmpRow[i];
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++)
					fDst[i] = tmpRow[i];
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++)
					dDst[i] = tmpRow[i];
				break;
			default:
				crError("unexpected type in put_row in pixel.c");
		}
	}
	else if (dstFormat == GL_DEPTH_COMPONENT) {
		switch (dstType) {
			case GL_BYTE:
				for (i = 0; i < width; i++)
					bDst[i] = FLOAT_TO_BYTE(tmpRow[i]);
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++)
					ubDst[i] = FLOAT_TO_UBYTE(tmpRow[i]);
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++)
					sDst[i] = FLOAT_TO_SHORT(tmpRow[i]);
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++)
					sDst[i] = FLOAT_TO_SHORT(tmpRow[i]);
				break;
			case GL_INT:
				for (i = 0; i < width; i++)
					bDst[i] = (GLbyte) FLOAT_TO_INT(tmpRow[i]);
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++)
					bDst[i] = (GLbyte) FLOAT_TO_UINT(tmpRow[i]);
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++)
					fDst[i] = tmpRow[i];
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++)
					dDst[i] = tmpRow[i];
				break;
			default:
				crError("unexpected type in put_row in pixel.c");
		}
	}
	else if (dstFormat == GL_RED || dstFormat == GL_GREEN ||
					 dstFormat == GL_BLUE || dstFormat == GL_ALPHA ||
					 dstFormat == GL_LUMINANCE || dstFormat == GL_INTENSITY) {
		int index;
		if (dstFormat == GL_RED)
			index = 0;
		else if (dstFormat == GL_LUMINANCE)
			index = 0;
		else if (dstFormat == GL_INTENSITY)
			index = 0;
		else if (dstFormat == GL_GREEN)
			index = 1;
		else if (dstFormat == GL_BLUE)
			index = 2;
		else
			index = 3;
		switch (dstType) {
			case GL_BYTE:
				for (i = 0; i < width; i++)
					bDst[i] = FLOAT_TO_BYTE(tmpRow[i*4+index]);
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++)
					ubDst[i] = FLOAT_TO_UBYTE(tmpRow[i*4+index]);
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++)
					sDst[i] = FLOAT_TO_SHORT(tmpRow[i*4+index]);
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++)
					usDst[i] = FLOAT_TO_USHORT(tmpRow[i*4+index]);
				break;
			case GL_INT:
				for (i = 0; i < width; i++)
					iDst[i] = FLOAT_TO_INT(tmpRow[i*4+index]);
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++)
					uiDst[i] = FLOAT_TO_UINT(tmpRow[i*4+index]);
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++)
					fDst[i] = tmpRow[i*4+index];
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++)
					dDst[i] = tmpRow[i*4+index];
				break;
			default:
				crError("unexpected type in put_row in pixel.c");
		}
	}
	else if (dstFormat == GL_LUMINANCE_ALPHA) {
		switch (dstType) {
			case GL_BYTE:
				for (i = 0; i < width; i++) {
					bDst[i*2+0] = FLOAT_TO_BYTE(tmpRow[i*4+0]);
					bDst[i*2+1] = FLOAT_TO_BYTE(tmpRow[i*4+3]);
				}
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++) {
					ubDst[i*2+0] = FLOAT_TO_UBYTE(tmpRow[i*4+0]);
					ubDst[i*2+1] = FLOAT_TO_UBYTE(tmpRow[i*4+3]);
				}
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++) {
					sDst[i*2+0] = FLOAT_TO_SHORT(tmpRow[i*4+0]);
					sDst[i*2+1] = FLOAT_TO_SHORT(tmpRow[i*4+3]);
				}
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++) {
					usDst[i*2+0] = FLOAT_TO_USHORT(tmpRow[i*4+0]);
					usDst[i*2+1] = FLOAT_TO_USHORT(tmpRow[i*4+3]);
				}
				break;
			case GL_INT:
				for (i = 0; i < width; i++) {
					iDst[i*2+0] = FLOAT_TO_INT(tmpRow[i*4+0]);
					iDst[i*2+1] = FLOAT_TO_INT(tmpRow[i*4+3]);
				}
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++) {
					uiDst[i*2+0] = FLOAT_TO_UINT(tmpRow[i*4+0]);
					uiDst[i*2+1] = FLOAT_TO_UINT(tmpRow[i*4+3]);
				}
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++) {
					fDst[i*2+0] = tmpRow[i*4+0];
					fDst[i*2+1] = tmpRow[i*4+3];
				}
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++) {
					dDst[i*2+0] = tmpRow[i*4+0];
					dDst[i*2+1] = tmpRow[i*4+3];
				}
				break;
			default:
				crError("unexpected type in put_row in pixel.c");
		}
	}
	else if (dstFormat == GL_RGB 
#ifdef CR_OPENGL_VERSION_1_2
					 || dstFormat == GL_BGR
#endif
					 ) {
		int r, b;
		if (dstFormat == GL_RGB) {
			r = 0; b = 2;
		}
		else {
			r = 2; b = 0;
		}
		switch (dstType) {
			case GL_BYTE:
				for (i = 0; i < width; i++) {
					bDst[i*3+r] = FLOAT_TO_BYTE(tmpRow[i*4+0]);
					bDst[i*3+1] = FLOAT_TO_BYTE(tmpRow[i*4+1]);
					bDst[i*3+b] = FLOAT_TO_BYTE(tmpRow[i*4+2]);
				}
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++) {
					ubDst[i*3+r] = FLOAT_TO_UBYTE(tmpRow[i*4+0]);
					ubDst[i*3+1] = FLOAT_TO_UBYTE(tmpRow[i*4+1]);
					ubDst[i*3+b] = FLOAT_TO_UBYTE(tmpRow[i*4+2]);
				}
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++) {
					sDst[i*3+r] = FLOAT_TO_SHORT(tmpRow[i*4+0]);
					sDst[i*3+1] = FLOAT_TO_SHORT(tmpRow[i*4+1]);
					sDst[i*3+b] = FLOAT_TO_SHORT(tmpRow[i*4+2]);
				}
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++) {
					usDst[i*3+r] = FLOAT_TO_USHORT(tmpRow[i*4+0]);
					usDst[i*3+1] = FLOAT_TO_USHORT(tmpRow[i*4+1]);
					usDst[i*3+b] = FLOAT_TO_USHORT(tmpRow[i*4+2]);
				}
				break;
			case GL_INT:
				for (i = 0; i < width; i++) {
					iDst[i*3+r] = FLOAT_TO_INT(tmpRow[i*4+0]);
					iDst[i*3+1] = FLOAT_TO_INT(tmpRow[i*4+1]);
					iDst[i*3+b] = FLOAT_TO_INT(tmpRow[i*4+2]);
				}
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++) {
					uiDst[i*3+r] = FLOAT_TO_UINT(tmpRow[i*4+0]);
					uiDst[i*3+1] = FLOAT_TO_UINT(tmpRow[i*4+1]);
					uiDst[i*3+b] = FLOAT_TO_UINT(tmpRow[i*4+2]);
				}
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++) {
					fDst[i*3+r] = tmpRow[i*4+0];
					fDst[i*3+1] = tmpRow[i*4+1];
					fDst[i*3+b] = tmpRow[i*4+2];
				}
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++) {
					dDst[i*3+r] = tmpRow[i*4+0];
					dDst[i*3+1] = tmpRow[i*4+1];
					dDst[i*3+b] = tmpRow[i*4+2];
				}
				break;
#ifdef CR_OPENGL_VERSION_1_2
			case GL_UNSIGNED_BYTE_3_3_2:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 7.0);
					int green = (int) (tmpRow[i*4+1] * 7.0);
					int blue  = (int) (tmpRow[i*4+b] * 3.0);
					ubDst[i] = (red << 5) | (green << 2) | blue;
				}
				break;
			case GL_UNSIGNED_BYTE_2_3_3_REV:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 7.0);
					int green = (int) (tmpRow[i*4+1] * 7.0);
					int blue  = (int) (tmpRow[i*4+b] * 3.0);
					ubDst[i] = red | (green << 3) | (blue << 6);
				}
				break;
			case GL_UNSIGNED_SHORT_5_6_5:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 31.0);
					int green = (int) (tmpRow[i*4+1] * 63.0);
					int blue  = (int) (tmpRow[i*4+b] * 31.0);
					usDst[i] = (red << 11) | (green << 5) | blue;
				}
				break;
			case GL_UNSIGNED_SHORT_5_6_5_REV:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 31.0);
					int green = (int) (tmpRow[i*4+1] * 63.0);
					int blue  = (int) (tmpRow[i*4+b] * 31.0);
					usDst[i] = (blue << 11) | (green << 5) | red;
				}
				break;
#endif
			default:
				crError("unexpected type in put_row in pixel.c");
		}
	}
	else if (dstFormat == GL_RGBA 
#ifdef CR_OPENGL_VERSION_1_2
					 || dstFormat == GL_BGRA 
#endif
					 ) {
		int r, b;
		if (dstFormat == GL_RGB) {
			r = 0; b = 2;
		}
		else {
			r = 2; b = 0;
		}
		switch (dstType) {
			case GL_BYTE:
				for (i = 0; i < width; i++) {
					bDst[i*4+r] = FLOAT_TO_BYTE(tmpRow[i*4+0]);
					bDst[i*4+1] = FLOAT_TO_BYTE(tmpRow[i*4+1]);
					bDst[i*4+b] = FLOAT_TO_BYTE(tmpRow[i*4+2]);
					bDst[i*4+3] = FLOAT_TO_BYTE(tmpRow[i*4+3]);
				}
				break;
			case GL_UNSIGNED_BYTE:
				for (i = 0; i < width; i++) {
					ubDst[i*4+r] = FLOAT_TO_UBYTE(tmpRow[i*4+0]);
					ubDst[i*4+1] = FLOAT_TO_UBYTE(tmpRow[i*4+1]);
					ubDst[i*4+b] = FLOAT_TO_UBYTE(tmpRow[i*4+2]);
					ubDst[i*4+3] = FLOAT_TO_UBYTE(tmpRow[i*4+3]);
				}
				break;
			case GL_SHORT:
				for (i = 0; i < width; i++) {
					sDst[i*4+r] = FLOAT_TO_SHORT(tmpRow[i*4+0]);
					sDst[i*4+1] = FLOAT_TO_SHORT(tmpRow[i*4+1]);
					sDst[i*4+b] = FLOAT_TO_SHORT(tmpRow[i*4+2]);
					sDst[i*4+3] = FLOAT_TO_SHORT(tmpRow[i*4+3]);
				}
				break;
			case GL_UNSIGNED_SHORT:
				for (i = 0; i < width; i++) {
					usDst[i*4+r] = FLOAT_TO_USHORT(tmpRow[i*4+0]);
					usDst[i*4+1] = FLOAT_TO_USHORT(tmpRow[i*4+1]);
					usDst[i*4+b] = FLOAT_TO_USHORT(tmpRow[i*4+2]);
					usDst[i*4+3] = FLOAT_TO_USHORT(tmpRow[i*4+3]);
				}
				break;
			case GL_INT:
				for (i = 0; i < width; i++) {
					iDst[i*4+r] = FLOAT_TO_INT(tmpRow[i*4+0]);
					iDst[i*4+1] = FLOAT_TO_INT(tmpRow[i*4+1]);
					iDst[i*4+b] = FLOAT_TO_INT(tmpRow[i*4+2]);
					iDst[i*4+3] = FLOAT_TO_INT(tmpRow[i*4+3]);
				}
				break;
			case GL_UNSIGNED_INT:
				for (i = 0; i < width; i++) {
					uiDst[i*4+r] = FLOAT_TO_UINT(tmpRow[i*4+0]);
					uiDst[i*4+1] = FLOAT_TO_UINT(tmpRow[i*4+1]);
					uiDst[i*4+b] = FLOAT_TO_UINT(tmpRow[i*4+2]);
					uiDst[i*4+3] = FLOAT_TO_UINT(tmpRow[i*4+3]);
				}
				break;
			case GL_FLOAT:
				for (i = 0; i < width; i++) {
					fDst[i*4+r] = tmpRow[i*4+0];
					fDst[i*4+1] = tmpRow[i*4+1];
					fDst[i*4+b] = tmpRow[i*4+2];
					fDst[i*4+3] = tmpRow[i*4+3];
				}
				break;
			case GL_DOUBLE:
				for (i = 0; i < width; i++) {
					dDst[i*4+r] = tmpRow[i*4+0];
					dDst[i*4+1] = tmpRow[i*4+1];
					dDst[i*4+b] = tmpRow[i*4+2];
					dDst[i*4+3] = tmpRow[i*4+3];
				}
				break;
#ifdef CR_OPENGL_VERSION_1_2
			case GL_UNSIGNED_SHORT_5_5_5_1:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 31.0);
					int green = (int) (tmpRow[i*4+1] * 31.0);
					int blue  = (int) (tmpRow[i*4+b] * 31.0);
					int alpha = (int) (tmpRow[i*4+3]);
					usDst[i] = (red << 11) | (green << 6) | (blue << 1) | alpha;
				}
				break;
			case GL_UNSIGNED_SHORT_1_5_5_5_REV:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 31.0);
					int green = (int) (tmpRow[i*4+1] * 31.0);
					int blue  = (int) (tmpRow[i*4+b] * 31.0);
					int alpha = (int) (tmpRow[i*4+3]);
					usDst[i] = (alpha << 15) | (blue << 10) | (green << 5) | red;
				}
				break;
			case GL_UNSIGNED_SHORT_4_4_4_4:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 15.0f);
					int green = (int) (tmpRow[i*4+1] * 15.0f);
					int blue  = (int) (tmpRow[i*4+b] * 15.0f);
					int alpha = (int) (tmpRow[i*4+3] * 15.0f);
					usDst[i] = (red << 12) | (green << 8) | (blue << 4) | alpha;
				}
				break;
			case GL_UNSIGNED_SHORT_4_4_4_4_REV:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 15.0f);
					int green = (int) (tmpRow[i*4+1] * 15.0f);
					int blue  = (int) (tmpRow[i*4+b] * 15.0f);
					int alpha = (int) (tmpRow[i*4+3] * 15.0f);
					usDst[i] = (alpha << 12) | (blue << 8) | (green << 4) | red;
				}
				break;
			case GL_UNSIGNED_INT_8_8_8_8:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 255.0f);
					int green = (int) (tmpRow[i*4+1] * 255.0f);
					int blue  = (int) (tmpRow[i*4+b] * 255.0f);
					int alpha = (int) (tmpRow[i*4+3] * 255.0f);
					uiDst[i] = (red << 24) | (green << 16) | (blue << 8) | alpha;
				}
				break;
			case GL_UNSIGNED_INT_8_8_8_8_REV:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 255.0f);
					int green = (int) (tmpRow[i*4+1] * 255.0f);
					int blue  = (int) (tmpRow[i*4+b] * 255.0f);
					int alpha = (int) (tmpRow[i*4+3] * 255.0f);
					uiDst[i] = (alpha << 24) | (blue << 16) | (green << 8) | red;
				}
				break;
			case GL_UNSIGNED_INT_10_10_10_2:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 1023.0f);
					int green = (int) (tmpRow[i*4+1] * 1023.0f);
					int blue  = (int) (tmpRow[i*4+b] * 1023.0f);
					int alpha = (int) (tmpRow[i*4+3] *    3.0f);
					uiDst[i] = (red << 22) | (green << 12) | (blue << 2) | alpha;
				}
				break;
			case GL_UNSIGNED_INT_2_10_10_10_REV:
				for (i = 0; i < width; i++) {
					int red   = (int) (tmpRow[i*4+r] * 1023.0f);
					int green = (int) (tmpRow[i*4+1] * 1023.0f);
					int blue  = (int) (tmpRow[i*4+b] * 1023.0f);
					int alpha = (int) (tmpRow[i*4+3] *    3.0f);
					uiDst[i] = (alpha << 30) | (blue << 20) | (green << 10) | red;
				}
				break;
#endif
			default:
			crError("unexpected type in put_row in pixel.c");
		}
	}
	else{
		crError("unexpected dest type in put_row in pixel.c");
	}
}


/**
 * Byte-swap an array of GLushorts
 */
static void
swap2(GLushort *us, GLuint n)
{
	GLuint i;
	for (i = 0; i < n; i++) {
		us[i] = (us[i] >> 8) | (us[i] << 8);
	}
}


/**
 * Byte-swap an array of GLuints
 */
static void
swap4(GLuint *ui, GLuint n)
{
	GLuint i;

	for (i = 0; i < n; i++) {
		GLuint b = ui[i];
		ui[i] =  (b >> 24)
				| ((b >> 8) & 0xff00)
				| ((b << 8) & 0xff0000)
				| ((b << 24) & 0xff000000);
	}
}


/**
 * Return number of bytes of storage needed to accomodate an
 * image with the given format, type, and size.
 * \return size in bytes or -1 if bad format or type
 */
unsigned int crImageSize( GLenum format, GLenum type, GLsizei width, GLsizei height )
{
	unsigned int bytes = width * height;

	if (type == GL_BITMAP)
	{
		/* This was wrong in the old code! */
		bytes = ((width + 7) / 8) * height;
	}
	else
	{
		bytes = width * height * crPixelSize( format, type );
	}

	return bytes;
}

/**
 * Return number of bytes of storage needed to accomodate a
 * 3D texture with the give format, type, and size.
 * \return size in bytes or -1 if bad format or type
 */
unsigned int crTextureSize( GLenum format, GLenum type, GLsizei width, GLsizei height, GLsizei depth )
{
        unsigned int bytes = width * height;

        if (type == GL_BITMAP)
        {
		/*
		 * Not sure about this one, so just multiply
		 * by the depth?
		 */
                bytes = ((width + 7) / 8) * height * depth;
        }
        else
        {
                bytes = width * height * depth * crPixelSize( format, type );
        }

        return bytes;
}

static const CRPixelPackState defaultPacking = {
	0, 		/* rowLength */
	0, 		/* skipRows */
	0, 		/* skipPixels */
	1, 		/* alignment */
	0, 		/* imageHeight */
	0, 		/* skipImages */
	GL_FALSE, 	/* swapBytes */
	GL_FALSE  	/* psLSBFirst */
};


void crPixelCopy1D( GLvoid *dstPtr, GLenum dstFormat, GLenum dstType,
										const GLvoid *srcPtr, GLenum srcFormat, GLenum srcType,
										GLsizei width, const CRPixelPackState *srcPacking )
{
	crPixelCopy2D( width, 1,
								 dstPtr, dstFormat, dstType, NULL,  /* dst */
								 srcPtr, srcFormat, srcType, srcPacking );  /* src */
}

void crPixelCopy2D( GLsizei width, GLsizei height,
										GLvoid *dstPtr, GLenum dstFormat, GLenum dstType,
										const CRPixelPackState *dstPacking,
										const GLvoid *srcPtr, GLenum srcFormat, GLenum srcType,
										const CRPixelPackState *srcPacking )
										
{
	const char *src = (const char *) srcPtr;
	char *dst = (char *) dstPtr;
	int srcBytesPerPixel;
	int dstBytesPerPixel;
	int srcBytesPerRow;
	int dstBytesPerRow;
 	int srcRowStrideBytes;
 	int dstRowStrideBytes;
	int bytesPerRow;
	int i;

	if (!dstPacking)
		 dstPacking = &defaultPacking;

	if (!srcPacking)
		 srcPacking = &defaultPacking;

	if (srcType == GL_BITMAP)
	{
		CRASSERT(dstType == GL_BITMAP);
		bytesPerRow = (width + 7) / 8;
		if (srcPacking->rowLength > 0)
			srcRowStrideBytes = (srcPacking->rowLength + 7) / 8;
		else
			srcRowStrideBytes = bytesPerRow;
		dstRowStrideBytes = bytesPerRow;

		for (i=0; i<height; i++) {
			crMemcpy( (void *) dst, (const void *) src, bytesPerRow );
			dst += dstRowStrideBytes;
			src += srcRowStrideBytes;
		} 
	}
	else
	{
		CRASSERT(dstType != GL_BITMAP);
		srcBytesPerPixel = crPixelSize( srcFormat, srcType );
		dstBytesPerPixel = crPixelSize( dstFormat, dstType );
		if (srcBytesPerPixel < 0 || dstBytesPerPixel < 0)
			return;

		/* Stride between rows (in bytes) */
		if (srcPacking->rowLength > 0)
			srcRowStrideBytes = srcPacking->rowLength * srcBytesPerPixel;
		else
			srcRowStrideBytes = width * srcBytesPerPixel;

		if (dstPacking->rowLength > 0)
			 dstRowStrideBytes = dstPacking->rowLength * dstBytesPerPixel;
		else
			 dstRowStrideBytes = width * dstBytesPerPixel;

		/* bytes per row */
		srcBytesPerRow = width * srcBytesPerPixel;
		dstBytesPerRow = width * dstBytesPerPixel;

		/* handle the alignment */
		if (srcPacking->alignment != 1) {
			i = ((long) src) % srcPacking->alignment;
			if (i)
				src += srcPacking->alignment - i;
			i = (long) srcRowStrideBytes % srcPacking->alignment;
			if (i)
				srcRowStrideBytes += srcPacking->alignment - i;
		}

		if (dstPacking->alignment != 1) {
			i = ((long) dst) % dstPacking->alignment;
			if (i)
				dst += dstPacking->alignment - i;
			i = (long) dstRowStrideBytes % dstPacking->alignment;
			if (i)
				dstRowStrideBytes += dstPacking->alignment - i;
		}

		/* handle skip rows */
		src += srcPacking->skipRows * srcRowStrideBytes;
		dst += dstPacking->skipRows * dstRowStrideBytes;

		/* handle skip pixels */
		src += srcPacking->skipPixels * srcBytesPerPixel;
		dst += dstPacking->skipPixels * dstBytesPerPixel;

		/* we don't do LSBFirst yet */
		if (srcPacking->psLSBFirst)
			crError( "Sorry, no lsbfirst for you" );
		if (dstPacking->psLSBFirst)
			crError( "Sorry, no lsbfirst for you" );

		if (srcFormat == dstFormat && srcType == dstType)
		{
			CRASSERT(srcBytesPerRow == dstBytesPerRow);
			for (i = 0; i < height; i++)
			{
				crMemcpy( (void *) dst, (const void *) src, srcBytesPerRow );
				/* check if src XOR dst swapping */
				if (srcPacking->swapBytes ^ dstPacking->swapBytes) {
					const GLint size = crSizeOfType(srcType);
					CRASSERT(srcType == dstType);
					if (size == 2) {
						swap2((GLushort *) dst, srcBytesPerRow / size);
					}
					else if (size == 4) {
						swap4((GLuint *)  dst, srcBytesPerRow / size);
					}
				}
				dst += dstRowStrideBytes;
				src += srcRowStrideBytes;
			}
		}
		else
		{
			/* need to do format and/or type conversion */
			char *swapRow = NULL;
			GLfloat *tmpRow = crAlloc( 4 * width * sizeof(GLfloat) );
			if (!tmpRow)
				crError("Out of memory in crPixelCopy2D");

			if (srcPacking->swapBytes) {
				swapRow = (char *) crAlloc(width * srcBytesPerPixel);
				if (!swapRow) {
					crError("Out of memory in crPixelCopy2D");
				}
			}

			for (i = 0; i < height; i++)
			{
				/* get src row as floats */
				if (srcPacking->swapBytes) {
					const GLint size = crSizeOfType(srcType);
					const GLint bytes = width * srcBytesPerPixel;
					crMemcpy(swapRow, src, bytes);
					if (size == 2)
						swap2((GLushort *) swapRow, bytes / 2);
					else if (size == 4)
						swap4((GLuint *) swapRow, bytes / 4);
					get_row(swapRow, srcFormat, srcType, width, tmpRow);
				}
				else {
					get_row(src, srcFormat, srcType, width, tmpRow);
				}

				/* store floats in dest row */
				if (dstPacking->swapBytes) {
					const GLint size = crSizeOfType(dstType);
					const GLint bytes = dstBytesPerPixel * width;
					put_row(dst, dstFormat, dstType, width, tmpRow);
					if (size == 2)
						swap2((GLushort *) dst, bytes / 2);
					else if (size == 4)
						swap4((GLuint *) dst, bytes / 4);
				}
				else {
					put_row(dst, dstFormat, dstType, width, tmpRow);
				}

				/* increment pointers for next row */
				dst += dstRowStrideBytes;
				src += srcRowStrideBytes;
			}

			crFree(tmpRow);
			if (swapRow)
				crFree(swapRow);
		}
	}
}

void crPixelCopy3D( GLsizei width, GLsizei height, GLsizei depth, 
                    GLvoid *dstPtr, GLenum dstFormat, GLenum dstType,
                    const CRPixelPackState *dstPacking,
                    const GLvoid *srcPtr, GLenum srcFormat, GLenum srcType,
                    const CRPixelPackState *srcPacking )

{
	int tex_size = 0;
	
	(void)srcPacking;
	(void)srcType;
	(void)srcFormat;
	(void)dstPacking;

	crWarning( "crPixelCopy3D:  simply crMemcpy'ing from srcPtr to dstPtr" );
	if (dstFormat != srcFormat)
		crWarning( "crPixelCopy3D: formats don't match!" );
	if (dstType != srcType)
		crWarning( "crPixelCopy3D: formats don't match!" );

	tex_size = crTextureSize( dstFormat, dstType, width, height, depth );
	crMemcpy( (void *) dstPtr, (void *) srcPtr, tex_size ); 
}

/* Round N up to the next multiple of 8 */
#define CEIL8(N)  (((N) + 7) & ~0x7)

void crBitmapCopy( GLsizei width, GLsizei height, GLubyte *dstPtr,
									 const GLubyte *srcPtr, const CRPixelPackState *srcPacking )
{
	if (srcPacking->psLSBFirst == GL_FALSE &&
			(srcPacking->rowLength == 0 || srcPacking->rowLength == width) &&
			srcPacking->skipRows == 0 &&
			srcPacking->skipPixels == 0 &&
			srcPacking->alignment == 1) {
		/* simple case */
		crMemcpy(dstPtr, srcPtr, CEIL8(width) * height / 8);
	}
	else {
		/* general case */
		const GLubyte *srcRow;
		const GLint dst_row_length = CEIL8(width) / 8;
		GLubyte *dstRow;
		GLint src_row_length;
		GLint i, j;

		if (srcPacking->rowLength > 0)
			src_row_length = srcPacking->rowLength;
		else 
			src_row_length = width;

		switch (srcPacking->alignment) {
			case 1:
				src_row_length = ( ( src_row_length + 7 ) & ~7 ) >> 3;
				break;
			case 2:
				src_row_length = ( ( src_row_length + 15 ) & ~15 ) >> 3;
				break;
			case 4:
				src_row_length = ( ( src_row_length + 31 ) & ~31 ) >> 3;
				break;
			case 8:
				src_row_length = ( ( src_row_length + 63 ) & ~63 ) >> 3;
				break;
			default:
				crError( "Invalid unpack alignment in crBitmapCopy");
				return;
		}

		/* src_row_length and dst_row_length are in bytes */

		srcRow = srcPtr + src_row_length * srcPacking->skipRows;
		dstRow = dstPtr;

		if (srcPacking->psLSBFirst) {
			for (j = 0; j < height; j++) {
				crMemZero(dstRow, dst_row_length);
				for (i = 0; i < width; i++) {
					const GLint iByte = (i + srcPacking->skipPixels) / 8;
					const GLint iBit  = (i + srcPacking->skipPixels) % 8;
					const GLubyte b = srcRow[iByte];
					if (b & (1 << iBit))
						dstRow[i / 8] |= (128 >> (i % 8));
				}
				srcRow += src_row_length;
				dstRow += dst_row_length;
			}
		}
		else {
			/* unpack MSB first */
			for (j = 0; j < height; j++) {
				crMemZero(dstRow, dst_row_length);
				for (i = 0; i < width; i++) {
					const GLint iByte = (i + srcPacking->skipPixels) / 8;
					const GLint iBit  = (i + srcPacking->skipPixels) % 8;
					const GLubyte b = srcRow[iByte];
					if (b & (128 >> iBit))
						dstRow[i / 8] |= (128 >> (i % 8));
				}
				srcRow += src_row_length;
				dstRow += dst_row_length;
			}
		}
	}
}
