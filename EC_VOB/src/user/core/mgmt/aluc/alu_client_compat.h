#ifndef _ALU_CLIENT_COMPAT_HEADER_H_
#define _ALU_CLIENT_COMPAT_HEADER_H_


#if !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif


#if !defined(va_copy) && defined(_MSC_VER)
#  define va_copy(dst,src) ((dst) = (src))
#endif


#if !defined(va_copy) && defined(__GNUC__) && __GNUC__ < 3
#  define va_copy(dst,src) __va_copy(dst, src)
#endif


#if !defined(_countof)
#  define _countof(_array) (sizeof(_array)/sizeof(*_array))
#endif


#endif /* _ALU_CLIENT_COMPAT_HEADER_H_ */
