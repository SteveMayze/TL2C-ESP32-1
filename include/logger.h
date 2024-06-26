/* 
 * File:   logger.h
 * Author: Steve Mayze
 *
 * Created on 23 April 2024
 */

#ifndef LOGGER_H
#define	LOGGER_H

 
#define LOGGER_LEVEL_OFF 0
#define LOGGER_LEVEL_ERROR 1
#define LOGGER_LEVEL_INFO 2
#define LOGGER_LEVEL_DEBUG 4
#define LOGGER_LEVEL_ALL 7

#define LOGGER_LEVEL LOGGER_LEVEL_INFO

#if defined(LOGGER_LEVEL) && LOGGER_LEVEL > LOGGER_LEVEL_OFF
    #define LOG_ERROR(args...) { Serial.print("ERROR: "); Serial.print(args); }
    #define LOG_ERROR_F(args...) { Serial.print("ERROR: "); Serial.printf(args); }
    #define LOG_ERROR_LN(args...) { Serial.print("ERROR: "); Serial.println(args); }
#else
    #define LOG_ERROR(args...)
    #define LOG_ERROR_F(args...)
    #define LOG_ERROR_LN(args...)
#endif

#if defined(LOGGER_LEVEL) && LOGGER_LEVEL > LOGGER_LEVEL_ERROR
    #define LOG_INFO(args...) { Serial.print("INFO: "); Serial.print(args); }
    #define LOG_INFO_F(args...) { Serial.print("INFO: "); Serial.printf(args); }
    #define LOG_INFO_LN(args...) { Serial.print("INFO: "); Serial.println(args); }
#else
    #define LOG_INFO(args...) 
    #define LOG_INFO_F(args...) 
    #define LOG_INFO_LN(args...) 
#endif


#if defined(LOGGER_LEVEL) && LOGGER_LEVEL > LOGGER_LEVEL_INFO 
    #define LOG_DEBUG(args...)  { Serial.print("DEBUG: "); Serial.print(args); }
    #define LOG_DEBUG_F(args...) { Serial.print("DEBUG: "); Serial.printf(args); }
    #define LOG_DEBUG_LN(args...) { Serial.print("DEBUG: "); Serial.println(args); }
#else
    #define LOG_DEBUG(args...) 
    #define LOG_DEBUG_F(args...) 
    #define LOG_DEBUG_LN(args...) 
#endif

    
#if defined(LOGGER_LEVEL) && LOGGER_LEVEL > LOGGER_LEVEL_INFO
    #define LOG_BYTE_STREAM(prefix, byte_stream, stream_size) { \
        LOG_INFO(prefix);                                       \
        for(int idx = 0; idx<stream_size; idx++) {              \
            Serial.printf("%02X ", byte_stream[idx]);           \
        }                                                       \
        Serial.print("\n");                                     \
    } 
    #else
        #define LOG_BYTE_STREAM(prefix, byte_stream, stream_size)
    #endif

    
#if defined(LOGGER_LEVEL) && LOGGER_LEVEL > LOGGER_LEVEL_INFO
    #define LOG_BIT_STREAM(value, bits) {      \
        int v = value, num_places = bits, mask = 0, n;                                            \
        for (n = 1; n <= num_places; n++)                           \
        {                                                           \
            mask = (mask << 1) | 0x0001;                            \
        }                                                           \
        v = v & mask;                                               \
        while (num_places)                                          \
        {                                                           \
            if (v & (0x0001 << num_places - 1))                     \
            {                                                       \
               Serial.print("1");                                   \
            }                                                       \
            else                                                    \
            {                                                       \
               Serial.print("0");                                   \
            }                                                       \
            --num_places;                                           \
            if (((num_places % 4) == 0) && (num_places != 0))       \
            {                                                       \
               Serial.print("_");                                   \
            }                                                       \
        }                                                           \
        Serial.print("\n");                                         \
    }

    #else
        #define LOG_BIT_STREAM(value, num_places)
    #endif
    
#endif	/* LOGGER_H */

