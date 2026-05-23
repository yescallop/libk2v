#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "k2v.h"

static uint8_t take_byte(const uint8_t *data, size_t size, size_t *offset)
{
	if (*offset < size) {
		uint8_t val = data[*offset];
		(*offset)++;
		return val;
	}
	return 0;
}

static uint32_t take_u32(const uint8_t *data, size_t size, size_t *offset)
{
	uint32_t v = 0;
	for (int i = 0; i < 4; i++) {
		v = (v << 8) | take_byte(data, size, offset);
	}
	return v;
}

static uint64_t take_u64(const uint8_t *data, size_t size, size_t *offset)
{
	uint64_t v = 0;
	for (int i = 0; i < 8; i++) {
		v = (v << 8) | (uint64_t)take_byte(data, size, offset);
	}
	return v;
}

static void make_token(const uint8_t *data, size_t size, size_t *offset, const char *alphabet, char *out, size_t out_size)
{
	size_t alpha_len = strlen(alphabet);
	if (out_size == 0) {
		return;
	}
	if (out_size == 1) {
		out[0] = '\0';
		return;
	}
	uint8_t len_byte = take_byte(data, size, offset);
	size_t len = (size_t)(len_byte % (out_size - 1)) + 1;
	if (len >= out_size) {
		len = out_size - 1;
	}
	for (size_t i = 0; i < len; i++) {
		uint8_t b = take_byte(data, size, offset);
		out[i] = alphabet[b % alpha_len];
	}
	out[len] = '\0';
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	k2v_stop_at_warning = false;
	k2v_show_warning = false;

	size_t offset = 0;

	const char *key_alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
	const char *val_alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_- .";

	char key_char[12];
	char key_int[12];
	char key_bool[12];
	char key_float[12];
	char key_long[12];
	char key_iarr[12];
	char key_farr[12];
	char key_sarr[12];

	char val_char[24];
	char val_s1[16];
	char val_s2[16];
	char val_s3[16];
	char val_s4[16];

	make_token(data, size, &offset, key_alphabet, key_char, sizeof(key_char));
	make_token(data, size, &offset, key_alphabet, key_int, sizeof(key_int));
	make_token(data, size, &offset, key_alphabet, key_bool, sizeof(key_bool));
	make_token(data, size, &offset, key_alphabet, key_float, sizeof(key_float));
	make_token(data, size, &offset, key_alphabet, key_long, sizeof(key_long));
	make_token(data, size, &offset, key_alphabet, key_iarr, sizeof(key_iarr));
	make_token(data, size, &offset, key_alphabet, key_farr, sizeof(key_farr));
	make_token(data, size, &offset, key_alphabet, key_sarr, sizeof(key_sarr));

	make_token(data, size, &offset, val_alphabet, val_char, sizeof(val_char));
	make_token(data, size, &offset, val_alphabet, val_s1, sizeof(val_s1));
	make_token(data, size, &offset, val_alphabet, val_s2, sizeof(val_s2));
	make_token(data, size, &offset, val_alphabet, val_s3, sizeof(val_s3));
	make_token(data, size, &offset, val_alphabet, val_s4, sizeof(val_s4));

	int int_val = (int)(int32_t)take_u32(data, size, &offset);
	long long_val = (long)(int64_t)take_u64(data, size, &offset);
	bool bool_val = (take_byte(data, size, &offset) & 1u) != 0;
	float float_val = (float)(int32_t)take_u32(data, size, &offset) / 1000.0f;

	int i_arr[4];
	float f_arr[4];
	char *s_arr[4] = { val_s1, val_s2, val_s3, val_s4 };

	int i_len = (int)(take_byte(data, size, &offset) % 4u);
	int f_len = (int)(take_byte(data, size, &offset) % 4u);
	int s_len = (int)(take_byte(data, size, &offset) % 4u);

	for (int i = 0; i < 4; i++) {
		i_arr[i] = (int)((int8_t)take_byte(data, size, &offset));
		f_arr[i] = (float)((int8_t)take_byte(data, size, &offset)) / 3.0f;
	}

	char *cfg = NULL;
	cfg = k2v_add_comment(cfg, "fuzz generated");
	cfg = k2v_add_config(char, cfg, key_char, val_char);
	cfg = k2v_add_config(int, cfg, key_int, int_val);
	cfg = k2v_add_config(bool, cfg, key_bool, bool_val);
	cfg = k2v_add_config(float, cfg, key_float, float_val);
	cfg = k2v_add_config(long, cfg, key_long, long_val);
	cfg = k2v_add_config(int_array, cfg, key_iarr, i_arr, i_len);
	cfg = k2v_add_config(float_array, cfg, key_farr, f_arr, f_len);
	cfg = k2v_add_config(char_array, cfg, key_sarr, s_arr, s_len);
	cfg = k2v_add_newline(cfg);

	if (cfg == NULL) {
		return 0;
	}

	(void)have_key(key_char, cfg);
	(void)have_key(key_int, cfg);
	(void)have_key(key_bool, cfg);
	(void)have_key(key_float, cfg);
	(void)have_key(key_long, cfg);
	(void)have_key(key_iarr, cfg);
	(void)have_key(key_farr, cfg);
	(void)have_key(key_sarr, cfg);

	char *s = key_get_char(key_char, cfg);
	if (s) {
		free(s);
	}
	(void)key_get_int(key_int, cfg);
	(void)key_get_bool(key_bool, cfg);
	(void)key_get_float(key_float, cfg);
	(void)key_get_long(key_long, cfg);

	int i_tmp[8];
	float f_tmp[8];
	char *s_tmp[8] = { 0 };
	(void)key_get_int_array(key_iarr, cfg, i_tmp, 8);
	(void)key_get_float_array(key_farr, cfg, f_tmp, 8);
	int s_tmp_len = key_get_char_array(key_sarr, cfg, s_tmp, 8);
	for (int i = 0; i < s_tmp_len; i++) {
		free(s_tmp[i]);
	}

	free(cfg);
	return 0;
}
