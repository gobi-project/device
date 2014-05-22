#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>
#include <libconfig.h>
#include <time.h>
#include <qrencode.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Blöcke
// 0x18000 - 0x18FFF Random Zugriff Block 1.1
// 0x19000 - 0x19FFF Random Zugriff Block 1.2
// 0x1A000 - 0x1AFFF Random Zugriff Block 2.1
// 0x1B000 - 0x1BFFF Random Zugriff Block 2.2
// 0x1C000 - 0x1CFFF Stack ohne Pop-Funktion
// 0x1D000 - 0x1DFFF Fehlermeldungen / SenML-Antworten
// 0x1E000 - 0x1EFFF MAC, UUID, PSK, PSK-Zeichen, ECC-Base-Point, Name, Model, Flashzeitpunkt
// 0x1F000 - 0x1FFFF Systemreserviert

//Read Only Fehlermeldungen / CoRE-Link- und SenML-Antworten
#define RES_B_ERR_05     0x1D000
#define LEN_B_ERR_05     73
#define RES_B_ERR_04     0x1D080
#define LEN_B_ERR_04     51
#define RES_B_ERR_03     0x1D100
#define LEN_B_ERR_03     53
#define RES_B_ERR_02     0x1D180
#define LEN_B_ERR_02     31
#define RES_B_ERR_01     0x1D200
#define LEN_B_ERR_01     61

#define RES_D_CORE       0x1D280
#define LEN_D_CORE       257
#define RES_SENML_LEDB   0x1D400
#define LEN_SENML_LEDB   42
#define RES_SENML_LEDD   0x1D440
#define LEN_SENML_LEDD   43
#define RES_SENML_TMP    0x1D480
#define LEN_SENML_TMP    48
#define RES_SENML_LUX    0x1D520
#define LEN_SENML_LUX    43
#define RES_SENML_HUM    0x1D560
#define LEN_SENML_HUM    46
#define RES_SENML_TMP_F  0x1D600
#define LEN_SENML_TMP_F  48
#define RES_SENML_VAL    0x1D640
#define LEN_SENML_VAL    44
#define RES_SENML_BUTTON 0x1D680
#define LEN_SENML_BUTTON 40
#define RES_SENML_RGB    0x1D720
#define LEN_SENML_RGB    43
#define RES_SENML_BOUT   0x1D760
#define LEN_SENML_BOUT   40

//Read Only Vars
#define RES_CONFIG       0x1E000
#define LEN_CONFIG       0x12
#define RES_UUID         0x1E020
#define LEN_UUID         0x10
#define RES_PSK          0x1E030
#define LEN_PSK          0x10
#define RES_ANSCHARS     0x1E040
#define LEN_ANSCHARS     0x40
#define RES_ECC_BASE_X   0x1E080
#define LEN_ECC_BASE_X   0x20
#define RES_ECC_BASE_Y   0x1E0A0
#define LEN_ECC_BASE_Y   0x20
#define RES_ECC_ORDER    0x1E0C0
#define LEN_ECC_ORDER    0x20
#define RES_NAME         0x1E0E0
#define LEN_NAME         0x40
#define RES_MODEL        0x1E120
#define LEN_MODEL        0x40
#define RES_FLASHTIME    0x1E160
#define LEN_FLASHTIME    0x04

char *anschars = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_-";

// ----------------------------------------------------------------------------

FILE *openFile(const char *name, const char *appendix, const char *mode);
void writeStandardConfig();
void writeImg(FILE *file, unsigned char *data, int width);

// ----------------------------------------------------------------------------

int main(int nArgs, char **argv) {
    unsigned int c, i, config;
    unsigned int buf[64];

    if (nArgs < 2) {
        writeStandardConfig();

        fprintf(stderr, "Mindestens eine Config erwartet: ./blaster <config.cfg> <config.cfg> <config.cfg> ...\n");
        fprintf(stderr, "Vorlage wurde in config.cfg erstellt.\n");

        exit(EXIT_FAILURE);
    }

    // qrdata = "UUID:PSK\0"
    char qrdata[54];
    qrdata[36] = ':';
    qrdata[53] = '\0';

    config_t cfg;
    for (config = 1; config < nArgs; config++) {
        config_init(&cfg);
        config_setting_t *setting;
        const char *str_val;
        if (access(argv[config], F_OK) == 0) {
            config_read_file(&cfg, argv[config]);
        } else {
            fprintf(stderr, "Unable to read config file.\n");
            exit(EXIT_FAILURE);
        }

        config_lookup_string(&cfg, "input", &str_val);
        FILE *in_bin = openFile(str_val, ".bin", "r");

        config_lookup_string(&cfg, "output", &str_val);
        FILE *out_bin = openFile(str_val, ".bin", "w");
        FILE *out_txt = openFile(str_val, ".txt", "w");
        FILE *out_pbm = openFile(str_val, ".pbm", "w");

        char output[131072];
        for (i = 8; (c = fgetc(in_bin)) != EOF; i++) {
            output[i] = (unsigned char) c;
        }

        // Ursprüngliche Länge der Firmware setzen im little Endian Encoding ---------
        unsigned int length = i - 8;
        memcpy(output + 4, (const void *) &length, 4);
        fprintf(out_txt, "Länge: %u = 0x%08x\n", length, length);

        // Rest zur initlialisierung zunächst mit 0x00 füllen
        for (; i < 0x1F000; i++) output[i] = 0x00;

        // Fehlermeldungen setzen -----------------------------------------------------
        char *buffer;
        buffer = "Ohne eine korrekte PSK-Uebertragung kann der PSK nicht ausgelesen werden.";
        memcpy(output + RES_B_ERR_05, buffer, LEN_B_ERR_05);
        buffer = "Der PSK wurde schon uebertragen. Eingabe ignoriert.";
        memcpy(output + RES_B_ERR_04, buffer, LEN_B_ERR_04);
        buffer = "Ohne gedrueckten Knopf wird der Pin nicht angenommen.";
        memcpy(output + RES_B_ERR_03, buffer, LEN_B_ERR_03);
        buffer = "Der PSK passt nicht zum Geraet.";
        memcpy(output + RES_B_ERR_02, buffer, LEN_B_ERR_02);
        buffer = "Der erste Teil des Handshakes wurde noch nicht durchgefuehrt.";
        memcpy(output + RES_B_ERR_01, buffer, LEN_B_ERR_01);

        // CoRE-Link- und SenML-Antworten setzen --------------------------------------
        buffer = "</d/name>;rt=\"dev.info\";if=\"core.rp\","
                 "</d/model>;rt=\"dev.info\";if=\"core.rp\","
                 "</d/uuid>;rt=\"dev.info\";if=\"core.rp\","
                 "</d/time>;rt=\"dev.info\";if=\"core.rp\","
                 "</d/psk>;rt=\"dev.info\";if=\"core.rp\","
                 "</d/route>;rt=\"dev.info\";if=\"core.rp\","
                 "</d/nb>;rt=\"dev.info\";if=\"core.rp\"";
        memcpy(output + RES_D_CORE, buffer, LEN_D_CORE);

        buffer = "{\"bn\":\"/led_b\",\"bu\":\"B\",\"e\":[{\"v\":\"%d\"}]}\x00";
        memcpy(output + RES_SENML_LEDB, buffer, LEN_SENML_LEDB);
        buffer = "{\"bn\":\"/led_d\",\"bu\":\"%%\",\"e\":[{\"v\":\"%d\"}]}\x00";
        memcpy(output + RES_SENML_LEDD, buffer, LEN_SENML_LEDD);
        buffer = "{\"bn\":\"/tmp\",\"bu\":\"%%degC\",\"e\":[{\"v\":\"%d.%d\"}]}\x00";
        memcpy(output + RES_SENML_TMP, buffer, LEN_SENML_TMP);
        buffer = "{\"bn\":\"/lux\",\"bu\":\"%%lx\",\"e\":[{\"v\":\"%d\"}]}\x00";
        memcpy(output + RES_SENML_LUX, buffer, LEN_SENML_LUX);
        buffer = "{\"bn\":\"/hum\",\"bu\":\"%%RH\",\"e\":[{\"v\":\"%d.%d\"}]}\x00";
        memcpy(output + RES_SENML_HUM, buffer, LEN_SENML_HUM);
        buffer = "{\"bn\":\"/tmp\",\"bu\":\"%%degF\",\"e\":[{\"v\":\"%d.%d\"}]}\x00";
        memcpy(output + RES_SENML_TMP_F, buffer, LEN_SENML_TMP_F);
        buffer = "{\"bn\":\"/val\",\"bu\":\"%%\",\"e\":[{\"v\":\"%d.%d\"}]}\x00";
        memcpy(output + RES_SENML_VAL, buffer, LEN_SENML_VAL);
        buffer = "{\"bn\":\"/btn\",\"bu\":\"B\",\"e\":[{\"v\":\"%d\"}]}\x00";
        memcpy(output + RES_SENML_BUTTON, buffer, LEN_SENML_BUTTON);
        buffer = "{\"bn\":\"/rgb\",\"bu\":\"ARGB\",\"e\":[{\"v\":\"%u\"}]}\x00";
        memcpy(output + RES_SENML_RGB, buffer, LEN_SENML_RGB);
        buffer = "{\"bn\":\"/swt\",\"bu\":\"B\",\"e\":[{\"v\":\"%d\"}]}\x00";
        memcpy(output + RES_SENML_BOUT, buffer, LEN_SENML_BOUT);

        // Contiki-Config setzen ------------------------------------------------------
        output[RES_CONFIG + 0] = 0x22;
        output[RES_CONFIG + 1] = 0x13;
        output[RES_CONFIG + 2] = 1;
        output[RES_CONFIG + 3] = 0;

        setting = config_lookup(&cfg, "eui");
        for (i = 0; i < 8; i++) {
          output[RES_CONFIG + 8 + i] = config_setting_get_int_elem(setting, 7 - i);
        }
        fprintf(out_txt,
                "EUI: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
                (uint8_t) output[RES_CONFIG + 15],
                (uint8_t) output[RES_CONFIG + 14],
                (uint8_t) output[RES_CONFIG + 13],
                (uint8_t) output[RES_CONFIG + 12],
                (uint8_t) output[RES_CONFIG + 11],
                (uint8_t) output[RES_CONFIG + 10],
                (uint8_t) output[RES_CONFIG +  9],
                (uint8_t) output[RES_CONFIG +  8]
        );

        output[RES_CONFIG + 16] = 15;
        output[RES_CONFIG + 17] = 17;
        output[RES_CONFIG + 18] = 0;
        output[RES_CONFIG + 19] = 0;
        output[RES_CONFIG + 20] = 5;
        output[RES_CONFIG + 21] = 0;
        output[RES_CONFIG + 22] = 0;
        output[RES_CONFIG + 23] = 0;

        // UUID setzen ----------------------------------------------------------------
        config_lookup_string(&cfg, "uuid", &str_val);
        memcpy(qrdata, str_val, 36);
        unsigned char uuid_bin[16];
        uuid_parse(str_val, uuid_bin);
        for (i = 0; i < 16; i++) output[RES_UUID + i] = uuid_bin[i];
        fprintf(out_txt, "UUID: %s\n", str_val);

        // PSK setzen -----------------------------------------------------------------
        config_lookup_string(&cfg, "psk", &str_val);
        memcpy(qrdata + 37, str_val, 16);
        for (i = 0; i < 16; i++) output[RES_PSK + i] = str_val[i];
        fprintf(out_txt, "PSK: %.*s\n", 16, str_val);

        // Zulässige Zeichen für Session und PSK setzen (alphanum Zeichen + "_" + "-")
        memcpy(output + RES_ANSCHARS, anschars, LEN_ANSCHARS);

        // ECC Base Points setzen -----------------------------------------------------
        uint32_t *base_x = (uint32_t *) (output + RES_ECC_BASE_X);
        base_x[0] = 0xd898c296;
        base_x[1] = 0xf4a13945;
        base_x[2] = 0x2deb33a0;
        base_x[3] = 0x77037d81;
        base_x[4] = 0x63a440f2;
        base_x[5] = 0xf8bce6e5;
        base_x[6] = 0xe12c4247;
        base_x[7] = 0x6b17d1f2;

        uint32_t *base_y = (uint32_t *) (output + RES_ECC_BASE_Y);
        base_y[0] = 0x37bf51f5;
        base_y[1] = 0xcbb64068;
        base_y[2] = 0x6b315ece;
        base_y[3] = 0x2bce3357;
        base_y[4] = 0x7c0f9e16;
        base_y[5] = 0x8ee7eb4a;
        base_y[6] = 0xfe1a7f9b;
        base_y[7] = 0x4fe342e2;

        uint32_t *order = (uint32_t *) (output + RES_ECC_ORDER);
        order[0] = 0xFC632551;
        order[1] = 0xF3B9CAC2;
        order[2] = 0xA7179E84;
        order[3] = 0xBCE6FAAD;
        order[4] = 0xFFFFFFFF;
        order[5] = 0xFFFFFFFF;
        order[6] = 0x00000000;
        order[7] = 0xFFFFFFFF;

        // Name setzen ----------------------------------------------------------------
        config_lookup_string(&cfg, "name", &str_val);
        snprintf(output + RES_NAME, LEN_NAME, "%s", str_val);
        fprintf(out_txt, "Name: %s\n", str_val);

        // Model setzen ---------------------------------------------------------------
        config_lookup_string(&cfg, "model", &str_val);
        snprintf(output + RES_MODEL, LEN_MODEL, "%s", str_val);
        fprintf(out_txt, "Model: %s\n", str_val);

        // Zeit setzen ----------------------------------------------------------------
        time_t my_time = time(NULL) + 37;
        memcpy(output + RES_FLASHTIME, (void *) &my_time, LEN_FLASHTIME);
        struct tm *timeinfo = localtime(&my_time);
        fwrite(buf, 1, strftime((char *) buf, 64, "Erzeugt am %d.%m.%Y um %H:%M:%S", timeinfo), out_txt);

        // Ausgeben -------------------------------------------------------------------
        for (i = 4; i < 0x1F000; i++) fputc(output[i], out_bin);

        // QR-Code generieren ---------------------------------------------------------
        QRcode *code = QRcode_encodeString8bit(qrdata, 3, QR_ECLEVEL_L);
        writeImg(out_pbm, code->data, code->width);

        fclose(in_bin);
        fclose(out_bin);
        fclose(out_txt);
        fclose(out_pbm);
    }

    exit(EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

FILE *openFile(const char *name, const char *appendix, const char *mode) {
    char filename[64];
    sprintf(filename, "%s%s", name, appendix);
    FILE *file = fopen(filename, mode);
    if (file == NULL) {
        perror("Wasn't able to open file.");
        exit(EXIT_FAILURE);
    }
    return file;
}

void writeStandardConfig() {
    unsigned int i;

    config_t cfg;
    config_init(&cfg);
    config_setting_t *setting;

    config_setting_t *root = config_root_setting(&cfg);

    setting = config_setting_add(root, "input", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, "dff_econotag");

    setting = config_setting_add(root, "output", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, "dff_e_econotag");

    uint8_t eui[8] = {0x02, 0x00, 0x00, 0x00, 0x60, 0xB1, 0x00, 0xFF};
    config_setting_t *array = config_setting_add(root, "eui", CONFIG_TYPE_ARRAY);
    for (i = 0; i < 8; ++i) {
        setting = config_setting_add(array, NULL, CONFIG_TYPE_INT);
        config_setting_set_format(setting, CONFIG_FORMAT_HEX);
        config_setting_set_int(setting, eui[i]);
    }

    unsigned char uuid_bin[16];
    uuid_generate(uuid_bin);
    char uuid[37];
    uuid_unparse(uuid_bin, uuid);
    setting = config_setting_add(root, "uuid", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, uuid);

    char psk[17];
    psk[16] = '\0';
    FILE *fd = fopen("/dev/urandom","r");
    if (fd == NULL) {
        perror("Öffnen von /dev/urandom fehlgeschlagen: ");
        return;
    }
    for (i = 0; i < 16; i++) {
        int c;
        while ((c = fgetc(fd)) == EOF);
        psk[i] = anschars[c % 64];
    }
    if (fclose(fd) == -1) {
        perror("Fehler beim Schließen von /dev/urandom\n");
    }
    setting = config_setting_add(root, "psk", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, psk);

    setting = config_setting_add(root, "name", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, "GOBI-Device");

    setting = config_setting_add(root, "model", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, "GOBI-ABCD-1234");

    config_write_file(&cfg, "config.cfg");
}

void writeImg(FILE *file, unsigned char *data, int width) {
    unsigned int buf[width];

    fprintf(file, "P4\n# %s\n%3u %3u\n", "QR-Code", width * 32, width * 32);
    
    int x, y;
    for (y = 0; y < width; y++) {
        for (x = 0; x < width; x++) {
            if (data[(y * width) + x] & 0x01) {
                buf[x] = 0xFFFFFFFF;
            } else {
                buf[x] = 0x00000000;
            }
        }
        for (x = 0; x < 32; x++) fwrite(buf, 4, width, file);
    }
}
