#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_LINE_LENGTH 4096
#define MAX_FILENAME_LENGTH 256
#define MIKA_VERSION "1.1.0"

typedef struct {
    char* input_file;
    char* output_file;
    int verbose;
    int keep_c_files;
    int line_number;
} TranslateContext;

void process_includes(char* line, FILE* output, const char* filename);
void process_print(char* line, FILE* output);
void process_return(char* line, FILE* output);
void process_comments(char* line);
void process_variables(char* line, FILE* output);
void process_function_declaration(char* line, FILE* output);
void process_input_function(char* line, FILE* output);
void process_power_function(char* line, FILE* output);
int is_mika_keyword(const char* word);
void show_help(void);
char* extract_function_name(const char* line);

void show_help(void) {
    printf("Mika Language Transpiler v%s\n", MIKA_VERSION);
    printf("Транслятор кода из Mika в C\n\n");
    printf("Использование: mika2c [ОПЦИИ] <входной_файл.mk>\n\n");
    printf("Опции:\n");
    printf("  -o <файл>    Указать выходной C файл\n");
    printf("  -k           Сохранять промежуточные .c файлы после компиляции\n");
    printf("  -v           Подробный вывод (verbose mode)\n");
    printf("  -h           Показать эту справку\n\n");
    printf("Примеры:\n");
    printf("  mika2c program.mk           # Транслировать program.mk в program.c\n");
    printf("  mika2c -o output.c prog.mk  # Транслировать в указанный файл\n");
    printf("  mika2c -v hello.mk          # Транслировать с подробным выводом\n");
}

int is_mika_keyword(const char* word) {
    const char* keywords[] = {
        "print",
        "input",
        "power",
        "return",
        "if",
        "else",
        "while",
        "for",
        "function",
        "var",
        "const",
        "true",
        "false",
        NULL
    };

    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

char* extract_function_name(const char* line) {
    const char* func_start = strstr(line, "function");
    if (func_start == NULL) {
        return NULL;
    }

    func_start += 8;
    while (*func_start && isspace(*func_start)) {
        func_start++;
    }

    const char* name_start = func_start;

    const char* name_end = name_start;
    while (*name_end && !isspace(*name_end) && *name_end != '(') {
        name_end++;
    }

    size_t name_length = name_end - name_start;
    if (name_length == 0) {
        return NULL;
    }

    char* function_name = malloc(name_length + 1);
    if (function_name == NULL) {
        perror("Ошибка выделения памяти");
        return NULL;
    }

    strncpy(function_name, name_start, name_length);
    function_name[name_length] = '\0';

    return function_name;
}

void process_includes(char* line, FILE* output, const char* filename) {
    if (strstr(line, "#include <System>")) {
        fprintf(output, "#include <stdio.h>\n");
        fprintf(output, "#include <stdlib.h>\n");
        fprintf(output, "#include <string.h>\n");
        fprintf(output, "#include <stdbool.h>\n");
        fprintf(output, "#include \"/usr/local/include/mika/mika_std.h\"\n\n");
    }
    else if (strstr(line, "#include <Math>")) {
        fprintf(output, "#include <math.h>\n");
    }
    else if (strstr(line, "#include <Time>")) {
        fprintf(output, "#include <time.h>\n");
    }
    else {
        fputs(line, output);
    }
}

void process_print(char* line, FILE* output) {
    char* pos = strstr(line, "print(");
    if (pos != NULL) {
        char result[MAX_LINE_LENGTH] = {0};
        char* before = line;
        char* after = pos + 5;

        strncpy(result, before, pos - before);
        result[pos - before] = '\0';
        strcat(result, "printf");
        strcat(result, after);

        fputs(result, output);
    } else {
        fputs(line, output);
    }
}

void process_input_function(char* line, FILE* output) {
    char* pos = strstr(line, "input()");
    if (pos != NULL) {
        fputs(line, output);
    } else {
        fputs(line, output);
    }
}

void process_power_function(char* line, FILE* output) {
    char* pos = strstr(line, "power(");
    if (pos != NULL) {
        fputs(line, output);
    } else {
        fputs(line, output);
    }
}

void process_return(char* line, FILE* output) {
    char* pos = strstr(line, "return 993");
    if (pos != NULL) {
        char result[MAX_LINE_LENGTH] = {0};
        char* before = line;

        strncpy(result, before, pos - before);
        result[pos - before] = '\0';
        strcat(result, "return 0");

        char* after = pos + 10;
        if (*after != '\0') {
            strcat(result, after);
        }

        fputs(result, output);
    } else {
        fputs(line, output);
    }
}

void process_comments(char* line) {
    char* comment = strstr(line, "//");
    if (comment != NULL) {
        *comment = '\0';

        if (comment > line && *(comment - 1) != '\n') {
            strcat(line, "\n");
        }
    }
}

void process_variables(char* line, FILE* output) {
    char* var_pos = strstr(line, "var ");
    if (var_pos != NULL) {
        char result[MAX_LINE_LENGTH] = {0};
        char* before = line;

        strncpy(result, before, var_pos - before);
        result[var_pos - before] = '\0';
        strcat(result, "int ");
        char* after = var_pos + 4;
        strcat(result, after);

        fputs(result, output);
    } else {
        fputs(line, output);
    }
}

void process_function_declaration(char* line, FILE* output) {
    char* func_pos = strstr(line, "function ");
    if (func_pos != NULL) {
        char result[MAX_LINE_LENGTH] = {0};
        char* before = line;

        strncpy(result, before, func_pos - before);
        result[func_pos - before] = '\0';
        strcat(result, "int ");
        char* after = func_pos + 9;
        strcat(result, after);

        fputs(result, output);
    } else {
        fputs(line, output);
    }
}

int main(int argc, char* argv[]) {
    TranslateContext ctx = {0};
    ctx.verbose = 0;
    ctx.keep_c_files = 0;
    ctx.line_number = 0;

    int opt;
    while ((opt = getopt(argc, argv, "o:kvh")) != -1) {
        switch (opt) {
            case 'o':
                ctx.output_file = optarg;
                break;
            case 'k':
                ctx.keep_c_files = 1;
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
        fprintf(stderr, " Ошибка: Не указан входной файл\n\n");
        show_help();
        return 1;
    }

    ctx.input_file = argv[optind];

    const char* ext = strrchr(ctx.input_file, '.');
    if (!ext || strcmp(ext, ".mk") != 0) {
        fprintf(stderr, " Ошибка: Файл должен иметь расширение .mk\n");
        fprintf(stderr, "   Получен: %s\n", ctx.input_file);
        return 1;
    }

    if (!ctx.output_file) {
        char base[MAX_FILENAME_LENGTH];
        strcpy(base, ctx.input_file);
        char* dot = strrchr(base, '.');
        if (dot) *dot = '\0';

        ctx.output_file = malloc(strlen(base) + 3);
        if (!ctx.output_file) {
            perror(" Ошибка выделения памяти");
            return 1;
        }
        sprintf((char*)ctx.output_file, "%s.c", base);
    }

    if (ctx.verbose) {
        printf("🔨 Начинаем трансляцию Mika -> C\n");
        printf("   Входной файл:  %s\n", ctx.input_file);
        printf("   Выходной файл: %s\n", ctx.output_file);
    }

    FILE* input = fopen(ctx.input_file, "r");
    if (!input) {
        perror(" Ошибка открытия входного файла");
        if (ctx.output_file && ctx.output_file != argv[optind]) {
            free((char*)ctx.output_file);
        }
        return 1;
    }

    FILE* output = fopen(ctx.output_file, "w");
    if (!output) {
        perror(" Ошибка создания выходного файла");
        fclose(input);
        if (ctx.output_file && ctx.output_file != argv[optind]) {
            free((char*)ctx.output_file);
        }
        return 1;
    }

    fprintf(output, "// ============================================================================\n");
    fprintf(output, "// Автоматически сгенерировано Mika Language Transpiler v%s\n", MIKA_VERSION);
    fprintf(output, "// Исходный файл: %s\n", ctx.input_file);
    fprintf(output, "// Время генерации: %s", __TIME__);
    fprintf(output, "// ВНИМАНИЕ: Этот файл создан автоматически, не редактируйте вручную!\n");
    fprintf(output, "// ============================================================================\n\n");

    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), input)) {
        ctx.line_number++;

        process_comments(line);

        if (line[0] == '\n' || line[0] == '\0') {
            fputc('\n', output);
            continue;
        }

        if (strstr(line, "#include")) {
            process_includes(line, output, ctx.input_file);
        }
        else if (strstr(line, "print(")) {
            process_print(line, output);
        }
        else if (strstr(line, "return 993")) {
            process_return(line, output);
        }
        else if (strstr(line, "function ")) {
            process_function_declaration(line, output);
        }
        else if (strstr(line, "var ")) {
            process_variables(line, output);
        }
        else if (strstr(line, "input()")) {
            process_input_function(line, output);
        }
        else if (strstr(line, "power(")) {
            process_power_function(line, output);
        }
        else {
            fputs(line, output);
        }
    }

    fclose(input);
    fclose(output);

    if (ctx.verbose) {
        printf(" Трансляция успешно завершена!\n");
        printf("   Обработано строк: %d\n", ctx.line_number);
        printf("   Результат: %s\n", ctx.output_file);
    }

    if (ctx.output_file && ctx.output_file != argv[optind]) {
        free((char*)ctx.output_file);
    }

    return 0;
}
