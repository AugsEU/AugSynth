#ifndef INC_CONSTANTS_H_
#define INC_CONSTANTS_H_

#define SAMPLE_RATE             32000
#define SAMPLE_PERIOD           (1.0f/SAMPLE_RATE)

#define A_2PI                   6.283185307f
#define A_PI                    3.14159265f

#define AUDIO_BUFF_LEN_DIV4     500
#define AUDIO_BUFF_LEN_DIV2     (2*AUDIO_BUFF_LEN_DIV4)
#define AUDIO_BUFF_LEN          (4*AUDIO_BUFF_LEN_DIV4)

#define RX_BUFF_LEN             5

#define MIDI_MAX				127.0f

#if defined   (__GNUC__)        /* GNU Compiler */
    #define __ALIGN    __attribute__ ((aligned (4)))
#endif /* __GNUC__ */

#endif /* INC_CONSTANTS_H_ */
