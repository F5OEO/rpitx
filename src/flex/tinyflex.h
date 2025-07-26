/*
 * tinyflex: A minimal, dependency-free, single-header library, FLEX encoder.
 * Written by Davidson Francis (aka Theldus) - 2025.
 *
 * This is free and unencumbered software released into the public domain.
 */

#ifndef TINYFLEX_H
#define TINYFLEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

/*
 * All section notes makes reference to the standard:
 * 'FLEX-TD RADIO PAGING SYSTEM - ARIB STANDARD - RCR STD-43A'
 *
 * Link:
 *   http://www.arib.or.jp/english/html/overview/doc/1-STD-43_A-E1.pdf
 */

/* BCH generator polynomial: x^10 + x^9 + x^8 + x^6 + x^5 + x^3 + 1. */
#define BCH_POLY     0x769
#define MESSAGE_BITS 21
#define CODE_BITS    32  /* 21 message + 10 ecc + 1 parity. */

#define BAUDRATE 1600
#define IW1 0         /* Idle word 1. */
#define IW2 0x1FFFFFU  /* Idle word 2. */

/* Amount of times to repeat the Emergency Re-Synchronization. */ 
#define ERS_AMOUNT 35

/* Amounts. */
#define BLOCKS_PER_FRAME 11
#define WORDS_PER_BLOCK  8   /* At 1600 bps. */
#define WORDS_PER_FRAME  (BLOCKS_PER_FRAME*WORDS_PER_BLOCK)

/* Number of words that can be used to store an alphanumeric encoded
 * message: Total words in frame - 4 (biw1 + shortaddress + alphanum vector)
 *
 * Since alpha num vector might be one or two words (short vs long addresses),
 * its better to play safe and reduce 1 word for both types.
 */
#define MAX_WORDS_ALPHA (WORDS_PER_FRAME - 4)

/* Since the first word in the alphanumeric message is flags and the second
 * word also contains 7 bits of flags, we should decrease in 4 the total number
 * of characters we can handle.
 */
#define MAX_CHARS_ALPHA (MAX_WORDS_ALPHA * 3) - 4

/* Section 3.2: Synchronization signal. */
static const uint8_t flex_bit_sync_1[] = {0xAA,0xAA,0xAA,0xAA};
static const uint8_t flex_bs[]         = {0xAA,0xAA};
static const uint8_t flex_bs_inv[]     = {0x55,0x55};
static const uint8_t flex_a1[]         = {0x78,0xF3,0x59,0x39};
static const uint8_t flex_a1_inv[]     = {0x87,0x0C,0xA6,0xC6};
static const uint8_t flex_ar[]         = {0xCB,0x20,0x59,0x39};
static const uint8_t flex_ar_inv[]     = {0x34,0xDF,0xA6,0xC6};
static const uint8_t flex_b[]          = {0x55,0x55};
static const uint8_t flex_cblock[]     = {0xAE,0xD8,0x45,0x12,0x7B};

/* Required size to store a FLEX message. */
#define FLEX_BUFFER_SIZE  \
  /* ERS. */              \
  (ERS_AMOUNT * (         \
  	sizeof(flex_bs) + sizeof(flex_ar) + sizeof(flex_bs_inv) + \
  	sizeof(flex_ar_inv))  \
  ) +            \
  /* S1. */      \
  (sizeof(flex_bit_sync_1) + sizeof(flex_a1) + sizeof(flex_b) + \
  	sizeof(flex_a1_inv)) + \
  /* FIW. */     \
  4 +            \
  /* S2. */      \
  (sizeof(flex_cblock)) +  \
  /* Single frame size. */ \
  WORDS_PER_FRAME*4

/* Return error code. */
#define TF_INVALID_MESSAGE    1
#define TF_INVALID_CAPCODE    2
#define TF_INVALID_FLEXBUFFER 3

/* Message configuration structure for extended API */
struct tf_message_config {
	uint8_t mail_drop;  /* 0 or 1 - Mail Drop Flag */
	/* Reserved for future flags */
};

/**
 * @brief Calculates the bit parity of a given 32-bit word provided in @p x.
 * @param x Word to be calculated the bit-parity.
 * @return  Returns 0 if the word have an odd parity, 1 if even.
 *
 * @note From 'Bit Twiddling Hacks', released into public domain.
 */
static uint8_t word_parity(uint32_t x) {
	x ^= x >> 16;
	x ^= x >> 8;
	x ^= x >> 4;
	x &= 0xf;
	return (0x6996 >> x) & 1;
}

/**
 * @brief Reverse bits in a 32-bit word.
 * @param v Word to have the bits swapped.
 * @return Returns a new word with all bits swapped:
 *         e.g., 1000 0000 0000 0000 0000 0000 1000 1011
 *   becomes:
 *               1101 0001 0000 0000 0000 0000 0000 0001
 *
 * @note From 'Bit Twiddling Hacks', released into public domain.
 */
static uint32_t rev32(uint32_t v) {
	/* Swap odd and even bits. */
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	/* Swap consecutive pairs. */
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	/* Swap nibbles. */
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	/* Swap bytes. */
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	/* Swap 2-byte long pairs. */
	v = (v >> 16) | (v << 16);
	return v;
}

/**
 * @brief Encodes a FLEX word: BCH(31,21) + even parity bit.
 * @param  dw  21-bit data to be encoded (in upper 21 bits of dw).
 * @return Returns a 32-bit codeword: [21 data bits][10 BCH][1 parity]
 *
 * @note BCH Generator Polynomial is described on Section 3.5.2.
 */
static uint32_t encode_word(uint32_t dw)
{
	uint32_t data, dividend, ecc, code31, parity;
	int i;

	/* Mask and extract top 21 bits */
	data = dw >> 11;

	/* Form dividend by shifting left 10 bits (space for parity bits). */
	dividend = data << 10;

	/* CRC-like division. */
	for (i = 30; i >= 10; i--) {
		if ((dividend >> i) & 1) {
			dividend ^= BCH_POLY << (i - 10);
		}
	}

	/* The 10-bit remainder is our BCH ECC. */
	ecc = dividend & 0x3FF;
	/* Assemble final 31-bit word: data (21 bits) + ecc (10 bits). */
	code31 = (data << 10) | ecc;
	/* Compute even parity bit over the 31 bits. */
	parity = word_parity(code31);

	/* Return full 32-bit codeword. */
	return (code31 << 1) | parity;
}

/**
 * @brief Calculates the FLEX checksum of a given data word.
 * @param dw Data word.
 * @return Returns the checksum.

 * @note Basic Word Structure, section 3.5.1.
 */
static uint32_t flex_checksum(uint32_t dw)
{
	uint32_t a,b,c,d,e;
	uint32_t csum;
	a = (dw >> 4)  & 0xF;
	b = (dw >> 8)  & 0xF;
	c = (dw >> 12) & 0xF;
	d = (dw >> 16) & 0xF;
	e = (dw >> 20) & 1;
	csum = (~(a+b+c+d+e)) & 0xF;
	return dw | csum;
}

/**
 * @brief Creates a FLEX Frame Information Word.
 *
 * @param cycle Cycle number (0-14).
 * @param frame Frame number (0-127).
 * @param n     Roaming network (1: roaming allowed / 0: not allowed)
 * @param r     Multiple transmission information, if any.
 * @param t     Low traffic flags.
 *
 * @return Returns a FIW properly encoded.
 *
 * @note Frame Information Word on section 3.6.
 */
static uint32_t
create_fiw(uint32_t cycle, uint32_t frame, uint32_t n, uint32_t r, uint32_t t)
{
	uint32_t dw;
	dw  = (cycle & 0xF)  << 4;
	dw |= (frame & 0x7F) << 8;
	dw |= (n & 1)        << 15;
	dw |= (r & 1)        << 16;
	dw |= (t & 0xF)      << 17;

	dw = flex_checksum(dw);
	return encode_word(rev32(dw));
}

/**
 * @brief Creates a FLEX Block Information Word 1.
 *
 * @param prio    Number of word representing priority addresses (0-15).
 * @param e_biw   End of BIW, more specifically, specifies when the Address
 *                Field starts (0-3, 2 bits).
 * @param s_vfield Specifies the offset of where the Vector Field starts (1-63)
 * @param carry   Two-bit flag that specifies if the info would be transmitted
 *                in subsequent frames: 0 no carry, 1-3 Carry on 1-3 Frames.
 * @param collapse Ranging from 0-7, specify when the pager should decode the
 *                 frames: 0 all frames, 1-7: 2^n cycle: 2^1 = Decodes every 2nd
 *                 frame, 2^2 = every 4th frame...
 *
 * @return Returns a filled BIW properly encoded.
 *                                      
 * @note Block Information Word on section 3.7.1.
 */
static uint32_t
create_biw1(uint32_t prio, uint32_t e_biw, uint32_t s_vfield, uint32_t carry,
	uint32_t collapse)
{
	uint32_t dw;
	dw  = (prio     & 0xF)  << 4;
	dw |= (e_biw    & 0x3)  << 8;
	dw |= (s_vfield & 0x3F) << 10;
	dw |= (carry    & 0x3)  << 16;
	dw |= (collapse  & 0x7)  << 18;

	dw = flex_checksum(dw);
	return encode_word(rev32(dw));
}

/**
 * @brief Creates a FLEX Alphanumeric Vector Word.
 *
 * @param msg_start Beginning of the message (relative to the block) (3-87)
 * @param msg_words Amount of words the message contains (2-85).
 *
 * @return Returns a filled Alphanum Vector Word properly encoded.
 *
 * @note Alphanum Vector Word on section 3.9.4.
 */
static uint32_t
create_alphanum_vector_word(uint32_t msg_start, uint32_t msg_words)
{
	uint32_t dw;
	dw  = (0x5 << 4);              /* 5 == Alpha Message Vector. */
	dw |= (msg_start & 0x7F) << 7;
	dw |= (msg_words & 0x7F) << 14;

	dw = flex_checksum(dw);
	return encode_word(rev32(dw));
}

/**
 * @brief Validates a given cap code if its a valid short address.
 * @param cap_code Code to be validated.
 * @return Returns 1 if valid, 0 otherwise.
 */
static int is_shortaddr_valid(uint64_t cap_code)
{
	/*
	 * According to Section 3.8, a short address is a 7-digit number that
	 * should fit into a single address, *and* be in the range 32769 and 1966080
	 * (inclusive).
	 *
	 * Appendix A (Section 5.1) also states that a CAPCODE should be add +32768
	 * in order to create an address.
	 */
	return (cap_code >= 1 && cap_code <= 1933312);
}

/**
 * @brief Creates a short address based on a given 7-digit cap code.
 *
 * @param cap_code 7-digit cap code to be encoded.
 *
 * @return Returns a short address, ranging from 1 to 1933312 (inclusive).
 *
 * @note Refer to 'Appendix A: CAPCODE'.
 */
static uint32_t create_short_address(uint32_t cap_code)
{
	uint32_t dw;
	dw = (cap_code + 32768) & 0x1FFFFF;
	dw = encode_word(rev32(dw));
	return dw;
}

/**
 * @brief Validates a given cap code if its a valid long address.
 * @param cap_code Code to be validated.
 * @return Returns 1 if valid, 0 otherwise.
 */
static int is_longaddr_valid(uint64_t cap_code) {
	return (cap_code >= 2101249ULL && cap_code <= 4297068542ULL);
}

/**
 * @brief Validates whether if a given cap code is valid or not.
 *
 * @param cap_code Capcode to be checked.
 * @param is_long  Pointer to indicate whether the capcode is
 *                 long or short.
 * @return Returns 1 if valid, 0 otherwise.
 */
static int is_capcode_valid(uint64_t cap_code, int *is_long) {
	*is_long = 0;
	if (is_shortaddr_valid(cap_code))
		return 1;
	else if (is_longaddr_valid(cap_code)) {
		*is_long = 1;
		return 1;
	}
	return 0;
}

/**
 * @brief Creates a long address based on a 9-10 digit cap code.
 *
 * @param cap_code 9-10 digit cap code to be encoded.
 * @param words    Pointer to the returned converted words.
 *
 * @return Returns 0 if success, -1 otherwise.
 *
 * @note Refer to 'Reference Document A: 5.15.5 CAPCODE to Binary Conversion'.
 */
static int create_long_address(uint64_t cap_code, uint32_t words[2])
{
	uint64_t result;
	uint32_t w1, w2;

	/* Set 1-2. */
	if (cap_code >= 2101249ULL && cap_code <= 1075843072ULL) {
		result = cap_code - 2068481ULL;
		w1 = (result % 32768) + 1;
		w2 = 2097151 - (result / 32768);
	}

	/* Sets 1-3 and 1-4. */
	else if (cap_code >= 1075843073ULL && cap_code <= 3223326720ULL) {
		result = cap_code - 2068481ULL;
		w1 = (result % 32768) + 1; /* Same as 1-2. */
		w2 = (result / 32768) + 1933312;
	}

	/* Set 2-3. */
	else if (cap_code >= 3223326721ULL && cap_code <= 4297068542ULL) {
		result = cap_code - 2068479ULL;
		w1 = (result % 32768) + 2064383;
		w2 = (result / 32768) + 1867776;
	}
	else
		return -1;

	words[0] = encode_word(rev32(w1));
	words[1] = encode_word(rev32(w2));
	return 0;
}

/**
 * @brief Do the block interleaving (Section 3.3) column wise, i.e.,
 *   row0  = word0bit0 word1bit0 ... word7bit0
 *   ...
 *   row31 = word0bit7 word1bit7 ... word7bit7
 *
 * The amount of data remains the same, but the each bit is transmitted
 * column-wise, i.e., a sequence of bits belongs to the next word,
 * instead of the same word. The reasoning for this is quite clever:
 * during burst errors (such as interference), we only corrupt a single
 * bit for each word, instead of possibly the entire word.
 *
 * Since our BCH can fix up to 2 wrong bits (per word), we can
 * gracefully recover from such bursts. I must admit, despite its
 * complexity (compared to POCSAG) FLEX is pretty awesome.
 *
 * @param block_num   Block number to interleave.
 * @param frame_words Pointer to our word list/all blocks.
 */
static void interleave_block(uint32_t block_num, uint32_t *frame_words)
{
	uint32_t src_block[8];
	uint8_t  dst_block[32];
	uint32_t i;

	memcpy(src_block, frame_words + block_num * 8, sizeof src_block);
	for (i = 0; i < 32; i++) {
		/* Since the words are already reversed, we grab the bits
		 * in the reversed other as well. */
		dst_block[i] =
			((src_block[0] >> (31 - i)) & 1) << 7 |
			((src_block[1] >> (31 - i)) & 1) << 6 |
			((src_block[2] >> (31 - i)) & 1) << 5 |
			((src_block[3] >> (31 - i)) & 1) << 4 |
			((src_block[4] >> (31 - i)) & 1) << 3 |
			((src_block[5] >> (31 - i)) & 1) << 2 |
			((src_block[6] >> (31 - i)) & 1) << 1 |
			((src_block[7] >> (31 - i)) & 1) << 0;
	}
	memcpy(frame_words + block_num * 8, dst_block, sizeof dst_block);
}

/**
 * @brief Encodes a given ASCII message into the a proper alphanumeric FLEX
 * message.
 *
 * @param frame_words [in/out] Words list for a given frame.
 * @param msg         [in]     ASCII message to be encoded.
 * @param msg_start            Block number in which the message starts inside
 *                             the frame.
 * @param fwc_p       [in/out] Frame word counter: keeps track of how many words
 *                             have been written until now.
 *
 * @note Message encoding described at: Reference Document A, Sec 3.8.8.3
 */
static void
create_alphanumeric_msg(uint32_t *frame_words, const char *msg,
	uint32_t msg_start, uint32_t *fwc_p, int is_long,
	const struct tf_message_config *config)
{
	uint32_t msg_word[MAX_WORDS_ALPHA] = {0};
	uint32_t word_idx;
	size_t max_len;
	size_t     len;
	uint32_t s_bit;
	uint32_t k_bit;
	int      shift;
	uint32_t     i;
	uint32_t   fwc;

	len      = strlen(msg);
	max_len  = (len > MAX_CHARS_ALPHA) ? MAX_CHARS_ALPHA : len;
	i        = 0;
	shift    = 7;
	word_idx = 1;

	/* Set bits f0f1 == 11 (per Sec 3.8.8.3), in order to indicate an initial
	 * fragment. */ 
	msg_word[0] = 0x1800;
	
	/* Set Mail Drop Flag if requested */
	if (config && config->mail_drop) {
		msg_word[0] |= (1 << 20);
	}

	/* Process characters. */
	while (i < max_len) {
		msg_word[word_idx] |= ((uint32_t)msg[i++] & 0x7F) << shift;
		shift += 7;

		/* Move to next word when current is full (3 chars per word). */
		if (shift == 21) {
			if (++word_idx >= MAX_WORDS_ALPHA)
				break;
			shift = 0;
		}
	}

	/* Do *not* let any unused 7-bit char in a word:
	 * The standard mandates to put an ASCII char ETX $03 on any unused chars
	 * inside a word.
	 */
	if (shift == 7) {
		msg_word[word_idx] |= ((0x3 << 7) | (0x3 << 14));
		word_idx++;
	}
	else if (shift == 14) {
		msg_word[word_idx] |= (0x3 << 14);
		word_idx++;
	}

	/* Calculates the S-bit: 7-bit signature field on second word (Sec 3.8.8.3)
	 */
	for (i = 1, s_bit = 0; i < word_idx; i++) {
		s_bit += (msg_word[i] & 0x7F);
		s_bit += (msg_word[i] >> 7)  & 0x7F;
		s_bit += (msg_word[i] >> 14) & 0x7F;
	}
	s_bit = ~s_bit;
	msg_word[1] |= s_bit & 0x7F;

	/* Calculate the K-bit: checksum over all words according to the
	 * specified bit positions on spec (Sec 3.8.8.3) */
	for (i = 0, k_bit = 0; i < word_idx; i++) {
		uint32_t g1 =  msg_word[i] & 0xFF;
		uint32_t g2 = (msg_word[i] >> 8)  & 0xFF;
		uint32_t g3 = (msg_word[i] >> 16) & 0x1F;
		k_bit += (g1+g2+g3);
	}
	k_bit = ~k_bit;
	msg_word[0] |= (k_bit & 0x3FF);

	/* == Add our data into the destination frame words. == */
	fwc = *fwc_p;

	/*
	 * The spec is *very* confusing here:
	 * On 'Reference Document A', 3.8.7.4. Alphanumeric Vector, it says:
	 *
	 *   Note: Long address results in second vector word which becomes the
	 *   first message word. Remaining message words in the message field is
	 *   reduced by 1.
	 *
	 * Our current approach already put the first message right after the vector
	 * word, so this is basically the same behavior used for short addresses.
	 *
	 * Our frame_words:
	 * Short address:
	 *    frame_words[0] = BIW
	 *    frame_words[1] = short address
	 *    frame_words[2] = alpha num vector
	 *    frame_words[3] = beginning of the message
	 *
	 * Long address:
	 *    frame_words[0] = BIW
	 *    frame_words[1] = long address first half
	 *    frame_words[2] = long address second half
	 *    frame_words[3] = alpha num vector
	 *    frame_words[4] = beginning of the message[0] / alpha num vector??
	 *    frame_words[5] = true beginning??
	 *
	 * When it says that my vector is 2 words and the second word is also the
	 * first message, implicitly it is saying that on my msg_start I should
	 * skip that idx too? i.e., start at 5 instead of 4... anyway, this works
	 * on my Motorola Advisor Elite.
	 */
	frame_words[fwc++] = create_alphanum_vector_word(msg_start + is_long,
		word_idx);
	for (i = 0; i < word_idx; i++)
		frame_words[fwc++] = encode_word(rev32(msg_word[i]));

	*fwc_p = fwc;
}


/* Writes a vector and a word into the specified buffer. */
#define SAVE_VEC(flex,vec) \
  do {\
  	memcpy((flex), (vec), sizeof((vec))); \
  	((flex)) += sizeof((vec)); \
  } while (0)

#define SAVE_WORD(flex,word) \
  do {\
    uint32_t w32 = (word); \
    (flex)[0] = (w32 >> 24) & 0xFF; \
    (flex)[1] = (w32 >> 16) & 0xFF; \
    (flex)[2] = (w32 >>  8) & 0xFF; \
    (flex)[3] =  w32        & 0xFF; \
    (flex) += 4; \
  } while (0)


/**
 * @brief Encodes an alphanumeric message given by @p msg, targeting
 * the given @p cap_code, with extended configuration options.
 *
 * @param msg       [in]  ASCII Message to be sent.
 * @param cap_code        A short or long address pager cap code.
 * @param flex_pckt [out] An output buffer that will holds the entire encoded
 *                        message. The user should provide a buffer of at least
 *                        FLEX_BUFFER_SIZE bytes.
 * @param flex_size       Output buffer size.
 * @param error     [out] Error flag pointer that indicates whether the
 *                        encoding was successful or not.
 * @param config    [in]  Optional configuration for message flags. Pass NULL
 *                        for defaults.
 *
 * @return Returns the number of bytes successfully written into the output
 *         or zero otherwise.
 */
size_t
tf_encode_flex_message_ex(const char *msg, uint64_t cap_code,
	uint8_t *flex_pckt, size_t flex_size, int *error,
	const struct tf_message_config *config)
{
	uint32_t frame_words[WORDS_PER_FRAME] = {0};
	uint8_t  *flex_pkt_ptr;
	uint32_t w[2];  /* Long address words.     */
	int   is_long;  /* Is cap-code long?.      */
	uint32_t  fwc;  /* Frame word counter.     */
	uint32_t    i;

	if (!error)
		return 0;

	*error = 0;

	if (!msg || *msg == '\0' || strlen(msg) > MAX_CHARS_ALPHA) {
		*error = -TF_INVALID_MESSAGE;
		return 0;
	}
	if (!is_capcode_valid(cap_code, &is_long)) {
		*error = -TF_INVALID_CAPCODE;
		return 0;
	}
	if (!flex_pckt || flex_size < FLEX_BUFFER_SIZE) {
		*error = -TF_INVALID_FLEXBUFFER;
		return 0;
	}

	fwc          = 0;
	flex_pkt_ptr = flex_pckt;

	/* Send ERS. */
	for (i = 0; i < ERS_AMOUNT; i++) {
		SAVE_VEC(flex_pkt_ptr, flex_bs);
		SAVE_VEC(flex_pkt_ptr, flex_ar);
		SAVE_VEC(flex_pkt_ptr, flex_bs_inv);
		SAVE_VEC(flex_pkt_ptr, flex_ar_inv);
	}

	/* =================== FRAME 0 =================== */

	/* Section 3.4 Transmission order: S1. */
	SAVE_VEC(flex_pkt_ptr, flex_bit_sync_1);
	SAVE_VEC(flex_pkt_ptr, flex_a1);
	SAVE_VEC(flex_pkt_ptr, flex_b);
	SAVE_VEC(flex_pkt_ptr, flex_a1_inv);

	/* Frame information word for frame 0. */
	SAVE_WORD(flex_pkt_ptr, create_fiw(0,0,0,0,0));

	/* S2: BS2 + C + inv.BS2 + inv.C */
	SAVE_VEC(flex_pkt_ptr, flex_cblock);

	/* BIW1 and address. */
	frame_words[fwc++] = create_biw1(0, 0, 2 + is_long, 0, 0);

	if (is_long) {
		create_long_address(cap_code, w);
		frame_words[fwc++] = w[0];
		frame_words[fwc++] = w[1];
	}
	else
		frame_words[fwc++] = create_short_address(cap_code);

	/* Create alphanumeric message. */
	create_alphanumeric_msg(frame_words, msg, 3 + is_long, &fwc, is_long, config);

	/* If our block is not fully filled yet, we should fill with
	 * idle blocks of all 1s and all 0s, per Section 3.4.1.
	 */
	for (; fwc < WORDS_PER_FRAME; fwc++) {
		if ((fwc % 2) == 0)
			frame_words[fwc] = 0xFFFFFFFF;
		else
			frame_words[fwc] = 0;
	}

	/* Block interleaving. */
	for (i = 0; i < BLOCKS_PER_FRAME; i++)
		interleave_block(i, frame_words);


	SAVE_VEC(flex_pkt_ptr, frame_words);
	return ((size_t)(flex_pkt_ptr - flex_pckt));
}

/**
 * @brief Encodes an alphanumeric message given by @p msg, targeting
 * the given @p cap_code.
 *
 * @param msg       [in]  ASCII Message to be sent.
 * @param cap_code        A short or long address pager cap code.
 * @param flex_pckt [out] An output buffer that will holds the entire encoded
 *                        message. The user should provide a buffer of at least
 *                        FLEX_BUFFER_SIZE bytes.
 * @param flex_size       Output buffer size.
 * @param error     [out] Error flag pointer that indicates whether the
 *                        encoding was successful or not.
 *
 * @return Returns the number of bytes successfully written into the output
 *         or zero otherwise.
 */
size_t
tf_encode_flex_message(const char *msg, uint64_t cap_code,
	uint8_t *flex_pckt, size_t flex_size, int *error)
{
	return tf_encode_flex_message_ex(msg, cap_code, flex_pckt, flex_size,
		error, NULL);
}

#if 0
int main(void)
{
	uint8_t vec[FLEX_BUFFER_SIZE] = {0};
	int error;
	(void)tf_encode_flex_message(
		"HACK THE PLANET", 1234567, vec, 800, &error);
	return 0;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* TINYFLEx_H */
