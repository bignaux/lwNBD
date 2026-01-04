/*


 */

#include <keyman.h>
#include <secrman.h>

/* TODO : move on ps2sdk once finished
 *
 *
 *
 */
// static int kelfbind(int port, int slot, const char *path)
static int kelfbind(int port, const char *path)
{
    /* verify right elf header */
    struct stat statbuf;
    FILE *fh = fopen(path, "rb");

    file_open(h, 0);
    if (fstat(h->fd, &statbuf) == -1) {
        //		nbdkit_error("fstat: %m");
        return -1;
    }

    statbuf.st_size;

    //	mcio_mcEncryptContentKey(buf, ContentKey);


    fh = fopen(card_kelf_path, "wb");
    if (fh == NULL)
        return -5;

    r = fwrite(buf, 1, filesize, fh);
    if (r != filesize)
        return -6;

    //	int CK_offset = mcio_mcGetKelfContentKeyOffset(buf);
    fseek(fh, CK_offset, SEEK_SET);

    r = fwrite(ContentKey, 1, 32, fh);
    if (r != 32)
        return -6;

    fclose(fh);

    free(buf);

    return 0;
}

static int mcinfo_cmd(void *handle)
{

    struct handle *h = handle;


    return 0;
}

static int mcman_query(void *handle, struct query_t *params, int nb_params)
{
    struct handle *h = handle;
    while (nb_params-- > 0) {
        if (0 == strcmp(params[nb_params].key, "info")) {
            mcinfo_cmd(handle);
        } else if (0 == strcmp(params[nb_params].key, "kelfbind")) {
            if (params[nb_params].val != NULL) {
                kelfbind(h->device, params[nb_params].val);
            }
        } else {
            lwnbd_debug("%s is not a known command.\n", params[nb_params].key);
        }
    }
    return 0;
}
