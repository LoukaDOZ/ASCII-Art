#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>

#define LENGTH 50
#define MEMORY_ERROR "Unable to allocate needed memory\n"
#define READ_PIXEL_ERROR "Error while reading pixel\n"
#define BIG_CHAR_TABLE "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "
#define SMALL_CHAR_TABLE "@%#*+=-:. "

// " .:-=+*#%@"
// "@%#*+=-:. "

// " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$"
// "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "

/* Retourne 1 si s est un commentaire, 0 sinon */
int is_comment(char* s) {
    if(strlen(s) < 1)
        return 0;

    if(s[0] == '#')
        return 1;

    return 0;
}

/* Lis width et height dans s et les place respectivement dans pw et ph */
void read_width_height(char* s, int* pw, int* ph) {
    sscanf(s, "%d %d", pw, ph);
}

/**
 * Lit et renvoie la première ligne de l'entrée standard qui n'est pas un commentaire d'un fichier PPM.
 * On suppose qu'un fichier PPM a été donné en entrée standard. 
 * Tous les commentaires ignorés de l'entrée standard sont consommés.
 *
 * La chaîne qui est renvoyée a été réservée avec malloc et peut être libérée avec free dès que son utilisation n'est plus requise.
 */
char* read_line(FILE* in){
    char * buffer = (char*) malloc(LENGTH * sizeof(char));

    if(buffer == NULL) {
        fprintf(stderr, MEMORY_ERROR);
        return NULL;
    }

    while(1){
        char* res = fgets(buffer, LENGTH * sizeof(char), in);

        if(res == NULL) {
            fprintf(stderr, "Unable to read PPM line\n");
            return NULL;
        }

        if(is_comment(buffer) == 1)
            continue;

        return buffer;
    }

}

/**
 * Renvoie vrai si c est un caractère représentant un chiffre et faux sinon
 */
int isnumber(char c){
    return c >= '0' && c <= '9';
}

int next_int_pixel(FILE* in) {
    char str[] = {'0', '0', '0', '\0'};
    char c;

    for(int i = -1; i < 3; i++) {
        c = fgetc(in);
        if(c == EOF) {
            fprintf(stderr, READ_PIXEL_ERROR);
            return -1;
        }

        if(c == '\n') {
            i--;
            continue;
        }

        if(i == -1 && isnumber(c) == 1) {
            str[0] = c;
            i++;
            continue;
        }

        str[i] = c;
    }

    char* end;
    int v = (int) strtol(str, &end, 10);

    if(v == 0 && (errno != 0 || str == end)) {
        fprintf(stderr, READ_PIXEL_ERROR);
        return -1;
    }

    return v;
}

/* lit une image ppm donnée en entrée standard 
et fait pointer pw et ph respectivement sur la largeur w et la hauteur h.
Renvoie un pointeur sur h pointeurs sur 3w entiers correspondant
aux différents pixels de l’image dans l’ordre */
int** read_ppm(FILE* in, int* pw, int* ph) {
    int** pixels;
    char* buffer;

    /* "P3" */
    buffer = read_line(in);
    if(buffer == NULL) {
        return NULL;
    }
    free(buffer);

    /* Width height */
    buffer = read_line(in);
    if(buffer == NULL) {
        return NULL;
    }
    read_width_height(buffer, pw, ph);
    free(buffer);

    /* Max color value */
    buffer = read_line(in);
    if(buffer == NULL) {
        return NULL;
    }
    free(buffer);

    /* Pixels */
    pixels = (int**) malloc(sizeof(int*) * (*ph));
    if(pixels == NULL) {
        fprintf(stderr, MEMORY_ERROR);
        return NULL;
    }

    for(int i = 0; i < *ph; i++) {
        pixels[i] = (int*) malloc(sizeof(int) * (*pw) * 3);
        if(pixels[i] == NULL) {
            fprintf(stderr, MEMORY_ERROR);
            return NULL;
        }

        for(int j = 0; j < *pw * 3; j++) {
            pixels[i][j] = next_int_pixel(in);

            if(pixels[i][j] < 0) {
                return NULL;
            }
        }
    }

    return pixels;
}

/* Renvoie x / 12.92 si x <= 0.04045, ((x+0.055)/1055)^2.4 sinon */
float lin(float x) {
    if(x <= 0.04045)
        return x / 12.92;
    
    return pow((x + 0.055) / 1.055, 2.4);
}

/* Renvoie 0.2126 * lin(r / 255) + 0.7152 * lin(g / 255) + 0.0722 * lin(b / 255) */
float mean(int r, int g, int b) {
    return 0.2126 * lin(((float) r) / 255) + 0.7152 * lin(((float) g) / 255) + 0.0722 * lin(((float) b) / 255);
}

/* Calcule m = mean(r, g, b)
Renvoie 12.92 ∗ m si m <= 0.0031308, 1.055 * (m^(1/2.4)) − 0.055 sinon */
float lum(int r, int g, int b) {
    float m = mean(r, g ,b);

    if(m <= 0.0031308)
        return m * 12.92;
    
    return pow(m, 1 / 2.4) * 1.055 - 0.055;
}

/* Renvoie un pointeur sur h pointeurs sur w entiers correspondant 
aux différents pixels de l’image dans l’ordre transformés en gris */
int** grayscale(int** pixels, int w, int h) {
    int** gray_pixels = (int**) malloc(sizeof(int*) * h);
    if(gray_pixels == NULL) {
        fprintf(stderr, MEMORY_ERROR);
        return NULL;
    }

    for(int i = 0; i < h; i++) {
        gray_pixels[i] = (int*) malloc(sizeof(int) * w);
        if(gray_pixels[i] == NULL) {
            fprintf(stderr, MEMORY_ERROR);
            return NULL;
        }

        for(int j = 0; j < w; j++) {
            int r = pixels[i][j * 3];
            int g = pixels[i][j * 3 + 1];
            int b = pixels[i][j * 3 + 2];

            /* De base arrondi à l'inférieur */
            gray_pixels[i][j] = (int) (255.0 * lum(r, g, b));
        }
    }

    return gray_pixels;
}

/* Renvoie la moyenne du niveau de gris de tous les pixels situés dans le rectangle 
entre les colonnes c et c + wr - 1 et entre les lignes l et l + hr - 1 */
float get_intensity(int** graypixels, int c, int l, int wr, int hr) {
    int c_end = c + wr - 1;
    int l_end = l + hr - 1;
    int sum = 0;
    int nb_elem = 1;

    for(int i = l; i <= l_end; i++) {
        for(int j = c; j <= c_end; j++) {
            sum += graypixels[i][j];
            nb_elem++;
        }
    }

    return (float) sum / (float) nb_elem;
}

/* Cette fonction découpe l’image en waa × haa rectangles
Elle renvoie un pointeur sur haa pointeurs sur waa flottants correspondant 
aux intensités de chaque rectangle */
float** get_intensities(int** graypixels, int w, int h, int waa, int haa) {
    float** intensities = (float**) malloc(sizeof(float*) * haa);
    if(intensities == NULL) {
        fprintf(stderr, MEMORY_ERROR);
        return NULL;
    }

    int rest_w = w;
    int rest_h = h;
    int rect_x = 0;
    int rect_y = 0;

    for(int i = 0; i < haa; i++) {
        intensities[i] = (float*) malloc(sizeof(float) * waa);
        if(intensities[i] == NULL) {
            fprintf(stderr, MEMORY_ERROR);
            return NULL;
        }

        int rect_h = floorf(((float) rest_h) / ((float) (haa - i)));
        rest_w = w;
        rect_x = 0;

        for(int j = 0; j < waa; j++) {
            int rect_w = floorf(((float) rest_w) / ((float) (waa - j)));

            intensities[i][j] = get_intensity(graypixels, rect_x, rect_y, rect_w, rect_h);
            rest_w -= rect_w;
            rect_x += rect_w;
        }

        rest_h -= rect_h;
        rect_y += rect_h;
    }

    return intensities;
}

/* Renvoie une intensité de gris convertie en charactère approprié issu de chartable */
char intensity_to_char(float intensity, char* chartable) {
    int charlen = strlen(chartable);
    float val = (1 - (intensity / 255)) * (float) (charlen - 1);
    int i = (int) round(val);

    return chartable[i];
}

/* Renvoie des listes de caratères des intensités converties */
char** intensities_to_chars(float** intensities, char* chartableW, int waa, int haa) {
    char** chars = (char**) malloc(sizeof(char*) * haa);
    if(chars == NULL) {
        fprintf(stderr, MEMORY_ERROR);
        return NULL;
    }

    for(int i = 0; i < haa; i++) {
        chars[i] = (char*) malloc(sizeof(char) * waa);
        if(chars[i] == NULL) {
            fprintf(stderr, MEMORY_ERROR);
            return NULL;
        }

        for(int j = 0; j < waa; j++) {
            chars[i][j] = intensity_to_char(intensities[i][j], chartableW);
        }
    }

    return chars;
}

void print_help(char* exec_name) {
    printf("Usage: %s <PPM image> <options>\n", exec_name);
    printf("/!\\ Warning: only PPM P3 images are allowed\n\n");

    printf("Options :\n");
    printf("-o <file>\tOutput file where the ASCII art is printed (default is stdout)\n");
    printf("-mw <size>\tASCII art max width\n");
    printf("-mh <size>\tASCII art max height\n");
    printf("-bc\t\tUse a big ASCII char table\n");
    printf("-sc\t\tUse a small ASCII char table (default)\n");
}

void print_arg_error(char* exec_name) {
    fprintf(stderr, "Usage: %s <PPM image> <options>\n", exec_name);
    fprintf(stderr, "Use %s -h for help\n", exec_name);
}

int args(int argc, char** argv, char** input, char** output, int* w, int* h, int* waa, int* haa, char** chartable) {
    if(argc < 2) {
        print_arg_error(argv[0]);
        return 0;
    }

    if(strcmp(argv[1], "-h") == 0) {
        print_help(argv[0]);
        return 0;
    }

    char* str;
    char* end;
    *input = argv[1];
    *output = NULL;
    *waa = -1;
    *haa = -1;
    *chartable = SMALL_CHAR_TABLE;

    for(int i = 2; i < argc; i++) {
        if(strcmp(argv[i], "-h") == 0) {
            print_help(argv[0]);
            return 0;
        } else if(strcmp(argv[i], "-o") == 0) {
            *output = argv[i + 1];
            i++;
        } else if(strcmp(argv[i], "-mw") == 0) {
            char* str = argv[i + 1];
            char* end;

            *waa = strtol(str, &end, 10);
            i++;

            if(*waa == 0 && (errno != 0 || str == end)) {
                fprintf(stderr, "Invalid size for -mw : %s\n", str);
                return 0;
            }
        } else if(strcmp(argv[i], "-mh") == 0) {
            char* str = argv[i + 1];
            char* end;
            
            *haa = strtol(str, &end, 10);
            i++;

            if(*haa == 0 && (errno != 0 || str == end)) {
                fprintf(stderr, "Invalid size for -mh : %s\n", str);
                return 0;
            }
        } else if(strcmp(argv[i], "-bc") == 0) {
            *chartable = BIG_CHAR_TABLE;
        } else if(strcmp(argv[i], "-sc") == 0) {
            *chartable = SMALL_CHAR_TABLE;
        } else {
            fprintf(stderr, "Unknwon argument %s\n", argv[i]);
            print_arg_error(argv[0]);
            return 0;
        }
    }

    return 1;
}

int main(int argc, char** argv) {
    int w, h, waa, haa;
    char* in_file;
    char* out_file;
    char* chartable;
    FILE* in;
    FILE* out;
    
    if(!args(argc, argv, &in_file, &out_file, &w, &h, &waa, &haa, &chartable))
        return EXIT_FAILURE;

    in = fopen(in_file, "r");
    if(in == NULL) {
        fprintf(stderr, "Failed to open %s", in_file);
        return EXIT_FAILURE;
    }

    int** ppm = read_ppm(in, &w, &h);
    fclose(in);

    if(ppm == NULL) {
        return EXIT_FAILURE;
    }

    if(waa <= 0 || waa > w)
        waa = w;
    
    if(haa <= 0 || haa > h)
        haa = h;

    int** gray_ppm = grayscale(ppm, w, h);
    if(gray_ppm == NULL) {
        return EXIT_FAILURE;
    }

    float** intensities = get_intensities(gray_ppm, w, h, waa, haa);
    if(intensities == NULL) {
        return EXIT_FAILURE;
    }

    char** chars = intensities_to_chars(intensities, chartable, waa, haa);
    if(chars == NULL) {
        return EXIT_FAILURE;
    }

    if (out_file != NULL) {
        out = fopen(out_file, "w");
        if(out == NULL) {
            fprintf(stderr, "Failed to open %s", out_file);
            return EXIT_FAILURE;
        }
    } else {
        out = stdout;
    }

    for(int i = 0; i < haa; i++) {
        for(int j = 0; j < waa; j++) {
            fprintf(out, "%c", chars[i][j]);
        }

        fprintf(out, "\n");
    }
    fprintf(out, "\n");

    if(out_file != NULL)
        fclose(out);

    free(ppm);
    free(gray_ppm);
    free(intensities);
    free(chars);

    return EXIT_SUCCESS;
}
