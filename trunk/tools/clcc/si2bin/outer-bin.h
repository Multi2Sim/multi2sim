

/*
 * AMD External ELF Binary
 */

struct si2bin_outer_bin_t
{
	/* ELF file create internally.
	 * Private field. */
	struct elf_enc_file_t *file;

	/* List of internal ELFs. Each element is of type 'si2bin_inner_bin_t'.
	 * Private field. */
	struct list_t *inner_bin_list;
};

struct si2bin_outer_bin_t *si2bin_outer_bin_create(void);
void si2bin_outer_bin_free(struct si2bin_outer_bin_t *outer_bin);

void si2bin_outer_bin_add(struct si2bin_outer_bin_t *outer_bin,
		struct si2bin_inner_bin_t *inner_bin);
void si2bin_outer_bin_generate(struct si2bin_outer_bin_t *outer_bin,
		struct elf_enc_buffer_t *buffer);


