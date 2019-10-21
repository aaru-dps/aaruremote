/*
 * Copyright (C) Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) Natalia Portillo
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * See the COPYRIGHT file distributed with this work for additional
 * information regarding copyright ownership.
 */

#ifndef DICREMOTE__ENDIAN_H_
#define DICREMOTE__ENDIAN_H_

#if HAVE_ENDIAN_H
#include <endian.h>
#elif HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#else

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321

#ifndef __BYTE_ORDER
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#ifndef bswap_16
#define bswap_16(x) ((uint16_t)((((uint16_t)(x)&0xff00) >> 8) | (((uint16_t)(x)&0x00ff) << 8)))
#endif /* !bswap_16 */

#ifndef bswap_32
#define bswap_32(x)                                                                                                    \
    ((uint32_t)((((uint32_t)(x)&0xff000000) >> 24) | (((uint32_t)(x)&0x00ff0000) >> 8) |                               \
                (((uint32_t)(x)&0x0000ff00) << 8) | (((uint32_t)(x)&0x000000ff) << 24)))
#endif /* !bswap_32 */

#ifndef bswap_64
#define bswap_64(x)                                                                                                    \
    ((uint64_t)((((uint64_t)(x)&0xff00000000000000ULL) >> 56) | (((uint64_t)(x)&0x00ff000000000000ULL) >> 40) |        \
                (((uint64_t)(x)&0x0000ff0000000000ULL) >> 24) | (((uint64_t)(x)&0x000000ff00000000ULL) >> 8) |         \
                (((uint64_t)(x)&0x00000000ff000000ULL) << 8) | (((uint64_t)(x)&0x0000000000ff0000ULL) << 24) |         \
                (((uint64_t)(x)&0x000000000000ff00ULL) << 40) | (((uint64_t)(x)&0x00000000000000ffULL) << 56)))
#endif /* !bswap_64 */

#ifndef htobe16
#if __BYTE_ORDER == __BIG_ENDIAN

#define htobe16(x) (x)
#define htole16(x) bswap_16(x)
#define be16toh(x) (x)
#define le16toh(x) bswap_16(x)

#else /* __BYTE_ORDER == __BIG_ENDIAN */

#define htobe16(x) bswap_16(x)
#define htole16(x) (x)
#define be16toh(x) bswap_16(x)
#define le16toh(x) (x)

#endif /* __BYTE_ORDER == __BIG_ENDIAN */
#endif /* !htobe16 */

#ifndef htobe32
#if __BYTE_ORDER == __BIG_ENDIAN

#define htobe32(x) (x)
#define htole32(x) bswap_32(x)
#define be32toh(x) (x)
#define le32toh(x) bswap_32(x)

#else /* __BYTE_ORDER == __BIG_ENDIAN */

#define htobe32(x) bswap_32(x)
#define htole32(x) (x)
#define be32toh(x) bswap_32(x)
#define le32toh(x) (x)

#endif /* __BYTE_ORDER == __BIG_ENDIAN */
#endif /* !htobe32 */

#ifndef htobe64
#if __BYTE_ORDER == __BIG_ENDIAN

#define htobe64(x) (x)
#define htole64(x) bswap_64(x)
#define be64toh(x) (x)
#define le64toh(x) bswap_64(x)

#else /* __BYTE_ORDER == __BIG_ENDIAN */

#define htobe64(x) bswap_64(x)
#define htole64(x) (x)
#define be64toh(x) bswap_64(x)
#define le64toh(x) (x)

#endif /* __BYTE_ORDER == __BIG_ENDIAN */
#endif /* !htobe64 */

#endif

#endif // DICREMOTE__ENDIAN_H_
