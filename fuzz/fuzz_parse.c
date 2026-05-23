#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "k2v.h"

static void make_key(const uint8_t *data, size_t size, char *out, size_t out_size)
{
	static const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
	size_t alpha_len = sizeof(alphabet) - 1;
	if (out_size == 0) {
		return;
	}
	if (out_size == 1) {
		out[0] = '\0';
		return;
	}
	size_t len = 1;
	if (size > 0) {
		len = (size_t)(data[0] % (out_size - 1)) + 1;
	}
	if (len >= out_size) {
		len = out_size - 1;
	}
	for (size_t i = 0; i < len; i++) {
		uint8_t b = (i < size) ? data[i] : (uint8_t)i;
		out[i] = alphabet[b % alpha_len];
	}
	out[len] = '\0';
}

static void exercise_buffer(const char *buf, const char *key)
{
	if (buf == NULL || key == NULL || key[0] == '\0') {
		return;
	}

	(void)have_key(key, buf);

	char *s = key_get_char(key, buf);
	if (s) {
		free(s);
	}

	(void)key_get_int(key, buf);
	(void)key_get_float(key, buf);
	(void)key_get_bool(key, buf);
	(void)key_get_long(key, buf);

	int i_arr[8];
	(void)key_get_int_array(key, buf, i_arr, 8);

	float f_arr[8];
	(void)key_get_float_array(key, buf, f_arr, 8);

	char *s_arr[8] = { 0 };
	int s_len = key_get_char_array(key, buf, s_arr, 8);
	for (int i = 0; i < s_len; i++) {
		free(s_arr[i]);
	}
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	k2v_stop_at_warning = false;
	k2v_show_warning = false;

	if (data == NULL || size == 0) {
		return 0;
	}

	char key[16];
	make_key(data, size, key, sizeof(key));

	char *raw = malloc(size + 1);
	if (!raw) {
		return 0;
	}
	memcpy(raw, data, size);
	raw[size] = '\0';

	char *san = malloc(size + 1);
	if (!san) {
		free(raw);
		return 0;
	}
	for (size_t i = 0; i < size; i++) {
		san[i] = (data[i] == 0) ? 'A' : (char)data[i];
	}
	san[size] = '\0';

	exercise_buffer(raw, key);
	exercise_buffer(raw, "key");
	exercise_buffer(raw, "some_key");

	exercise_buffer(san, key);
	exercise_buffer(san, "key");

	free(san);
	free(raw);
	return 0;
}
