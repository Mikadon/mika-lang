#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MIKA_VERSION "1.1.0"
#define MAX_FILENAME_LENGTH 256
#define MAX_CMD_LENGTH 512

typedef struct {
    char* input_file;
    char* output_file;
    int verbose;
    int debug;
    int keep_files;
    int compile_only;
    char* compiler_flags;
} CompileContext;

void show_help(void);
int file_exists(const char* filename);
int execute_command(const char* cmd, int verbose);
int compile_mika(CompileContext* ctx);
void cleanup_files(const char* c_file, const char* o_file, int keep_files);

void show_help(void) {
    printf("🐧 Mika Language Compiler v%s\n", MIKA_VERSION);
    printf("Использование: mikac [ОПЦИИ] <файл.mk>\n\n");
    printf("Опции:\n");
    printf("  -o <файл>    Указать имя выходного исполняемого файла\n");
    printf("  -c           Только компиляция, без линковки\n");
    printf("  -g           Включить отладочную информацию\n");
    printf("  -k           Сохранять промежуточные файлы\n");
    printf("  -v           Подробный вывод\n");
    printf("  -h           Показать эту справку\n");
}

int file_exists(const char* filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

int execute_command(const char* cmd, int verbose) {
    if (verbose) {
        printf("💻 Выполняем: %s\n", cmd);
    }
    int result = system(cmd);
    if (verbose) {
        if (result == 0) {
            printf("   ✅ Команда выполнена успешно\n");
        } else {
            printf("   ❌ Ошибка выполнения команды (код: %d)\n", result);
        }
    }
    return result;
}

void cleanup_files(const char* c_file, const char* o_file, int keep_files) {
    if (!keep_files) {
        if (c_file && file_exists(c_file)) {
            remove(c_file);
        }
        if (o_file && file_exists(o_file)) {
            remove(o_file);
        }
    }
}

char* create_temp_stdlib(void) {
    char* temp_file = malloc(MAX_FILENAME_LENGTH);
    if (!temp_file) return NULL;

    strcpy(temp_file, "/tmp/mika_std_");

    char random_part[10];
    sprintf(random_part, "%d", rand() % 10000);
    strcat(temp_file, random_part);
    strcat(temp_file, ".c");

    FILE* lib = fopen(temp_file, "w");
    if (!lib) {
        free(temp_file);
        return NULL;
    }

    fprintf(lib, "#include <stdio.h>\n");
    fprintf(lib, "#include <string.h>\n");
    fprintf(lib, "#include <stdlib.h>\n\n");

    fprintf(lib, "int input(void) {\n");
    fprintf(lib, "    int value;\n");
    fprintf(lib, "    scanf(\"%%d\", &value);\n");
    fprintf(lib, "    return value;\n");
    fprintf(lib, "}\n\n");

    fprintf(lib, "int power(int base, int exponent) {\n");
    fprintf(lib, "    int result = 1;\n");
    fprintf(lib, "    for (int i = 0; i < exponent; i++) {\n");
    fprintf(lib, "        result *= base;\n");
    fprintf(lib, "    }\n");
    fprintf(lib, "    return result;\n");
    fprintf(lib, "}\n\n");

    fprintf(lib, "int absolute(int number) {\n");
    fprintf(lib, "    return (number < 0) ? -number : number;\n");
    fprintf(lib, "}\n\n");

    fprintf(lib, "int* array_create(int size) {\n");
    fprintf(lib, "    return (int*)malloc(size * sizeof(int));\n");
    fprintf(lib, "}\n\n");

    fprintf(lib, "void array_free(int* array) {\n");
    fprintf(lib, "    free(array);\n");
    fprintf(lib, "}\n\n");

    fprintf(lib, "int array_size(int* array, int size) {\n");
    fprintf(lib, "    return size;\n");
    fprintf(lib, "}\n\n");

    fprintf(lib, "void input_string(char* buffer, int size) {\n");
    fprintf(lib, "    int c;\n");
    fprintf(lib, "    while ((c = getchar()) != '\\n' && c != EOF);\n");
    fprintf(lib, "    fgets(buffer, size, stdin);\n");
    fprintf(lib, "    size_t len = strlen(buffer);\n");
    fprintf(lib, "    if (len > 0 && buffer[len-1] == '\\n') {\n");
    fprintf(lib, "        buffer[len-1] = '\\0';\n");
    fprintf(lib, "    }\n");
    fprintf(lib, "}\n");

    fclose(lib);
    return temp_file;
}

int compile_mika(CompileContext* ctx) {
    char c_file[MAX_FILENAME_LENGTH] = {0};
    char o_file[MAX_FILENAME_LENGTH] = {0};

    strcpy(c_file, ctx->input_file);
    char* dot = strrchr(c_file, '.');
    if (dot) *dot = '\0';
    strcat(c_file, ".c");

    strcpy(o_file, ctx->input_file);
    dot = strrchr(o_file, '.');
    if (dot) *dot = '\0';
    strcat(o_file, ".o");

    if (ctx->verbose) {
        printf("\n🚀 Этап 1: Трансляция Mika -> C\n");
    }

    char transpile_cmd[MAX_CMD_LENGTH];
    if (ctx->verbose) {
        sprintf(transpile_cmd, "mika2c -v '%s'", ctx->input_file);
    } else {
        sprintf(transpile_cmd, "mika2c '%s'", ctx->input_file);
    }

    if (execute_command(transpile_cmd, ctx->verbose) != 0) {
        fprintf(stderr, "❌ Ошибка на этапе трансляции Mika -> C\n");
        return 1;
    }

    if (!file_exists(c_file)) {
        fprintf(stderr, "❌ Не удалось создать C файл: %s\n", c_file);
        return 1;
    }

    if (ctx->verbose) {
        printf("\n🔧 Этап 2: Компиляция C -> объектный файл\n");
    }

    char compile_cmd[MAX_CMD_LENGTH];
    if (ctx->debug) {
        sprintf(compile_cmd, "gcc -c %s -o %s -g -I/usr/local/include", c_file, o_file);
    } else {
        sprintf(compile_cmd, "gcc -c %s -o %s -I/usr/local/include", c_file, o_file);
    }

    if (execute_command(compile_cmd, ctx->verbose) != 0) {
        fprintf(stderr, "❌ Ошибка компиляции C кода\n");
        cleanup_files(c_file, NULL, ctx->keep_files);
        return 1;
    }

    if (!file_exists(o_file)) {
        fprintf(stderr, "❌ Не удалось создать объектный файл: %s\n", o_file);
        cleanup_files(c_file, NULL, ctx->keep_files);
        return 1;
    }

    if (ctx->compile_only) {
        if (ctx->verbose) {
            printf("\n✅ Компиляция завершена (создан %s)\n", o_file);
        }
        if (!ctx->keep_files) {
            remove(c_file);
        }
        return 0;
    }

    if (ctx->verbose) {
        printf("\n🔗 Этап 3: Компиляция стандартной библиотеки\n");
    }

    char lib_o_file[MAX_FILENAME_LENGTH] = "/tmp/mika_std.o";
    char* lib_c_file = NULL;
    char* temp_lib_file = NULL;

    if (file_exists("/usr/local/include/mika/mika_std.c")) {
        lib_c_file = "/usr/local/include/mika/mika_std.c";
        if (ctx->verbose) {
            printf("   Используем системную библиотеку: %s\n", lib_c_file);
        }
    } else {
        temp_lib_file = create_temp_stdlib();
        if (temp_lib_file) {
            lib_c_file = temp_lib_file;
            if (ctx->verbose) {
                printf("   Создана временная библиотека: %s\n", lib_c_file);
            }
        } else {
            fprintf(stderr, "❌ Не удалось создать временную библиотеку\n");
            cleanup_files(c_file, o_file, ctx->keep_files);
            return 1;
        }
    }

    char lib_compile_cmd[MAX_CMD_LENGTH];
    sprintf(lib_compile_cmd, "gcc -c %s -o %s", lib_c_file, lib_o_file);

    if (execute_command(lib_compile_cmd, ctx->verbose) != 0) {
        fprintf(stderr, "❌ Ошибка компиляции стандартной библиотеки\n");
        cleanup_files(c_file, o_file, ctx->keep_files);
        if (temp_lib_file) {
            remove(temp_lib_file);
            free(temp_lib_file);
        }
        return 1;
    }

    if (temp_lib_file) {
        remove(temp_lib_file);
        free(temp_lib_file);
    }

    if (!ctx->output_file) {
        ctx->output_file = malloc(strlen(ctx->input_file) + 1);
        if (!ctx->output_file) {
            fprintf(stderr, "❌ Ошибка выделения памяти\n");
            cleanup_files(c_file, o_file, ctx->keep_files);
            remove(lib_o_file);
            return 1;
        }
        strcpy((char*)ctx->output_file, ctx->input_file);
        dot = strrchr((char*)ctx->output_file, '.');
        if (dot) *dot = '\0';
    }

    if (ctx->verbose) {
        printf("   Выходной файл: %s\n", ctx->output_file);
    }

    if (ctx->verbose) {
        printf("\n🔗 Этап 4: Линковка исполняемого файла\n");
    }

    char link_cmd[MAX_CMD_LENGTH];
    if (ctx->debug) {
        sprintf(link_cmd, "gcc %s %s -o '%s' -g", o_file, lib_o_file, ctx->output_file);
    } else {
        sprintf(link_cmd, "gcc %s %s -o '%s'", o_file, lib_o_file, ctx->output_file);
    }

    if (execute_command(link_cmd, ctx->verbose) != 0) {
        fprintf(stderr, "❌ Ошибка линковки\n");
        cleanup_files(c_file, o_file, ctx->keep_files);
        remove(lib_o_file);
        return 1;
    }

    remove(lib_o_file);

    cleanup_files(c_file, o_file, ctx->keep_files);

    if (ctx->verbose) {
        printf("\n🎉 Компиляция успешно завершена!\n");
        printf("   Создан исполняемый файл: %s\n", ctx->output_file);

        struct stat st;
        if (stat(ctx->output_file, &st) == 0) {
            printf("   Размер файла: %ld байт\n", st.st_size);
        }
    } else {
        printf("✅ Успешно: %s -> %s\n", ctx->input_file, ctx->output_file);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    CompileContext ctx = {0};
    ctx.compiler_flags = "";
    ctx.verbose = 0;
    ctx.debug = 0;
    ctx.keep_files = 0;
    ctx.compile_only = 0;

    int opt;
    while ((opt = getopt(argc, argv, "o:cgkvh")) != -1) {
        switch (opt) {
            case 'o':
                ctx.output_file = optarg;
                break;
            case 'c':
                ctx.compile_only = 1;
                break;
            case 'g':
                ctx.debug = 1;
                break;
            case 'k':
                ctx.keep_files = 1;
                break;
            case 'v':
                ctx.verbose = 1;
                break;
            case 'h':
                show_help();
                return 0;
            default:
                show_help();
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "❌ Ошибка: Не указан входной файл\n\n");
        show_help();
        return 1;
    }

    ctx.input_file = argv[optind];

    if (!file_exists(ctx.input_file)) {
        fprintf(stderr, "❌ Ошибка: Файл '%s' не существует\n", ctx.input_file);
        return 1;
    }

    const char* ext = strrchr(ctx.input_file, '.');
    if (!ext || strcmp(ext, ".mk") != 0) {
        fprintf(stderr, "❌ Ошибка: Файл должен иметь расширение .mk\n");
        return 1;
    }

    if (ctx.verbose) {
        printf("🐧 Mika Language Compiler v%s\n", MIKA_VERSION);
        printf("📁 Компилируем файл: %s\n", ctx.input_file);
    }

    int result = compile_mika(&ctx);

    if (ctx.output_file && ctx.output_file != argv[optind]) {
        free((char*)ctx.output_file);
    }

    return result;
}
