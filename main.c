#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(path) _mkdir(path)
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #define MKDIR(path) mkdir(path, 0755)
#endif

#include "sqlite3.h"

#define MAX_NOME 100
#define MAX_EMAIL 100
#define MAX_SENHA 32
#define PATH_TRABALHOS "dados/trabalhos/"

sqlite3 *db;

/* --- Funções utilitárias --- */
void linha(char *b, int s) {
    if (fgets(b, s, stdin)) {
        size_t n = strlen(b);
        if (n > 0 && b[n-1] == '\n') b[n-1] = '\0';
    }
}

void lower_str(char *s) {
    for (int i = 0; s[i]; ++i) s[i] = tolower((unsigned char)s[i]);
}

void garantirPastas() {
    MKDIR("dados");
    MKDIR(PATH_TRABALHOS);
}

/* --- Inicialização do banco --- */
void inicializarBanco() {
    garantirPastas();
    if (sqlite3_open("dados/escola.db", &db)) {
        printf("Erro ao abrir banco: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS alunos ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " nome TEXT NOT NULL,"
        " email TEXT UNIQUE NOT NULL,"
        " senha TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS professores ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " nome TEXT NOT NULL,"
        " email TEXT UNIQUE NOT NULL,"
        " senha TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS notas ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " id_aluno INTEGER NOT NULL UNIQUE,"
        " prova1 REAL DEFAULT 0,"
        " prova2 REAL DEFAULT 0,"
        " trabalho_filename TEXT,"
        " trabalho_nota REAL DEFAULT 0,"
        " FOREIGN KEY(id_aluno) REFERENCES alunos(id) ON DELETE CASCADE"
        ");";

    char *err = 0;
    if (sqlite3_exec(db, sql, 0, 0, &err) != SQLITE_OK) {
        printf("Erro SQL inicializar: %s\n", err);
        sqlite3_free(err);
    }
}

/* --- Cadastro de aluno e professor --- */
void cadAluno() {
    char nome[MAX_NOME], email[MAX_EMAIL], senha[MAX_SENHA];
    printf("Nome: "); linha(nome, MAX_NOME);
    printf("Email: "); linha(email, MAX_EMAIL);
    printf("Senha: "); linha(senha, MAX_SENHA);

    const char *sql = "INSERT INTO alunos (nome, email, senha) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, senha, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE)
        printf("Aluno cadastrado com sucesso.\n");
    else
        printf("Erro ao cadastrar aluno (email pode ja existir).\n");

    sqlite3_finalize(stmt);
}

void cadProf() {
    char nome[MAX_NOME], email[MAX_EMAIL], senha[MAX_SENHA];
    printf("Nome: "); linha(nome, MAX_NOME);
    printf("Email: "); linha(email, MAX_EMAIL);
    printf("Senha: "); linha(senha, MAX_SENHA);

    const char *sql = "INSERT INTO professores (nome, email, senha) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, senha, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE)
        printf("Professor cadastrado com sucesso.\n");
    else
        printf("Erro ao cadastrar professor (email pode ja existir).\n");

    sqlite3_finalize(stmt);
}

/* --- Login --- */
int loginAluno(int *out_id) {
    char email[MAX_EMAIL], senha[MAX_SENHA];
    printf("Email: "); linha(email, MAX_EMAIL);
    printf("Senha: "); linha(senha, MAX_SENHA);

    const char *sql = "SELECT id, nome FROM alunos WHERE email=? AND senha=?;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, senha, -1, SQLITE_STATIC);

    int r = sqlite3_step(stmt);
    if (r == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *nome = sqlite3_column_text(stmt, 1);
        printf("Bem vindo (aluno): %s\n", nome);
        if (out_id) *out_id = id;
        sqlite3_finalize(stmt);
        return id;
    } else {
        printf("Login invalido.\n");
        sqlite3_finalize(stmt);
        return -1;
    }
}

int loginProf() {
    char email[MAX_EMAIL], senha[MAX_SENHA];
    printf("Email: "); linha(email, MAX_EMAIL);
    printf("Senha: "); linha(senha, MAX_SENHA);

    const char *sql = "SELECT id, nome FROM professores WHERE email=? AND senha=?;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, senha, -1, SQLITE_STATIC);

    int r = sqlite3_step(stmt);
    if (r == SQLITE_ROW) {
        const unsigned char *nome = sqlite3_column_text(stmt, 1);
        printf("Bem vindo (professor): %s\n", nome);
        sqlite3_finalize(stmt);
        return 1;
    } else {
        printf("Login invalido.\n");
        sqlite3_finalize(stmt);
        return -1;
    }
}

/* --- Upload de arquivo txt --- */
int copiarArquivoParaDados(const char *origem, const char *destino) {
    FILE *f_in = fopen(origem, "rb");
    if (!f_in) return 0;
    FILE *f_out = fopen(destino, "wb");
    if (!f_out) { fclose(f_in); return 0; }

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f_in)) > 0) {
        fwrite(buf, 1, n, f_out);
    }
    fclose(f_in);
    fclose(f_out);
    return 1;
}

void postarTrabalho(int idAluno) {
    char caminhoOrigem[256];
    printf("Caminho do arquivo .txt: ");
    linha(caminhoOrigem, sizeof(caminhoOrigem));

    time_t t = time(NULL);
    char nomeDestino[256];
    sprintf(nomeDestino, PATH_TRABALHOS "aluno%d_%ld.txt", idAluno, (long)t);

    if (!copiarArquivoParaDados(caminhoOrigem, nomeDestino)) {
        printf("Erro ao copiar arquivo.\n");
        return;
    }

    const char *sql = "INSERT OR REPLACE INTO notas (id_aluno, trabalho_filename) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, idAluno);
    sqlite3_bind_text(stmt, 2, nomeDestino, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE)
        printf("Trabalho enviado com sucesso!\n");
    else
        printf("Erro ao registrar trabalho.\n");
    sqlite3_finalize(stmt);
}

/* --- Boletim do aluno --- */
void verBoletimAluno(int idAluno) {
    const char *sql = "SELECT prova1, prova2, trabalho_nota, trabalho_filename FROM notas WHERE id_aluno = ?;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, idAluno);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        double p1 = sqlite3_column_double(stmt, 0);
        double p2 = sqlite3_column_double(stmt, 1);
        double trab = sqlite3_column_double(stmt, 2);
        const unsigned char *fname = sqlite3_column_text(stmt, 3);

        double media = p1 * 0.4 + p2 * 0.4 + trab * 0.2;
        const char *status = (media >= 7.0) ? "Aprovado" : "Reprovado";

        printf("\n--- Boletim ---\n");
        printf("Prova1: %.2f | Prova2: %.2f | Trabalho: %.2f\nArquivo: %s\nMedia final: %.2f | %s\n",
               p1, p2, trab, fname ? (const char*)fname : "(nenhum)", media, status);
    } else {
        printf("Nenhuma nota encontrada.\n");
    }
    sqlite3_finalize(stmt);
}

/* --- Professor: lancar notas (CORRIGIDO) --- */
void lancarNotasProfessor() {
    char email[MAX_EMAIL];
    printf("Email do aluno: ");
    linha(email, MAX_EMAIL);

    const char *sql_id = "SELECT id FROM alunos WHERE email=?;";
    sqlite3_stmt *stmt_id;
    sqlite3_prepare_v2(db, sql_id, -1, &stmt_id, NULL);
    sqlite3_bind_text(stmt_id, 1, email, -1, SQLITE_STATIC);

    int r = sqlite3_step(stmt_id);
    if (r != SQLITE_ROW) {
        printf("Aluno nao encontrado.\n");
        sqlite3_finalize(stmt_id);
        return;
    }
    int idAluno = sqlite3_column_int(stmt_id, 0);
    sqlite3_finalize(stmt_id);

    double p1, p2, trab;
    printf("Nota Prova 1 (0-10): "); scanf("%lf", &p1); getchar();
    printf("Nota Prova 2 (0-10): "); scanf("%lf", &p2); getchar();
    printf("Nota Trabalho (0-10): "); scanf("%lf", &trab); getchar();

    const char *sql_check = "SELECT COUNT(*) FROM notas WHERE id_aluno=?;";
    sqlite3_stmt *stmt_check;
    sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, NULL);
    sqlite3_bind_int(stmt_check, 1, idAluno);
    sqlite3_step(stmt_check);
    int existe = sqlite3_column_int(stmt_check, 0);
    sqlite3_finalize(stmt_check);

    sqlite3_stmt *stmt;
    if (existe) {
        const char *sql_update = "UPDATE notas SET prova1=?, prova2=?, trabalho_nota=? WHERE id_aluno=?;";
        sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL);
        sqlite3_bind_double(stmt, 1, p1);
        sqlite3_bind_double(stmt, 2, p2);
        sqlite3_bind_double(stmt, 3, trab);
        sqlite3_bind_int(stmt, 4, idAluno);
    } else {
        const char *sql_insert = "INSERT INTO notas (id_aluno, prova1, prova2, trabalho_nota) VALUES (?, ?, ?, ?);";
        sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
        sqlite3_bind_int(stmt, 1, idAluno);
        sqlite3_bind_double(stmt, 2, p1);
        sqlite3_bind_double(stmt, 3, p2);
        sqlite3_bind_double(stmt, 4, trab);
    }

    if (sqlite3_step(stmt) == SQLITE_DONE)
        printf("Notas lancadas com sucesso!\n");
    else
        printf("Erro ao lancar notas: %s\n", sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
}

/* --- Chatbot da secretaria --- */
void chatbotSecretaria() {
    char pergunta[200];
    printf("\n=== Chatbot da Secretaria ===\nDigite 'sair' para voltar.\n");
    while (1) {
        printf("\nVoce: ");
        linha(pergunta, sizeof(pergunta));
        if (strcmp(pergunta, "sair") == 0) break;
        char p[200]; strncpy(p, pergunta, sizeof(p)); lower_str(p);
        if (strstr(p, "horario")) printf("Chatbot: A secretaria funciona das 8h as 18h.\n");
        else if (strstr(p, "nota")) printf("Chatbot: As notas sao lancadas pelos professores e vistas no boletim.\n");
        else if (strstr(p, "trabalho")) printf("Chatbot: O aluno pode enviar o trabalho em arquivo .txt.\n");
        else if (strstr(p, "turma")) printf("Chatbot: Cada aluno pertence a uma turma registrada no sistema.\n");
        else printf("Chatbot: Nao entendi. Pergunte sobre horario, nota, trabalho ou turma.\n");
    }
}

/* --- Menus --- */
void menuAluno(int idAluno) {
    int o;
    do {
        printf("\n-- AREA DO ALUNO --\n1 - Ver boletim\n2 - Postar trabalho\n0 - Voltar\nEscolha: ");
        scanf("%d", &o); getchar();
        if (o == 1) verBoletimAluno(idAluno);
        else if (o == 2) postarTrabalho(idAluno);
    } while (o != 0);
}

void menuProfessor() {
    int o;
    do {
        printf("\n-- AREA DO PROFESSOR --\n1 - Lancar notas\n0 - Voltar\nEscolha: ");
        scanf("%d", &o); getchar();
        if (o == 1) lancarNotasProfessor();
    } while (o != 0);
}

void menuSecretaria() {
    int o;
    do {
        printf("\n-- SECRETARIA --\n1 - Falar com chatbot\n2 - Ver telefone\n0 - Voltar\nEscolha: ");
        scanf("%d", &o); getchar();
        if (o == 1) chatbotSecretaria();
        else if (o == 2) printf("Telefone: (11) 4400-1132\n");
    } while (o != 0);
}

/* --- Menu principal --- */
void menu() {
    int o;
    do {
        printf("\n=== MENU PRINCIPAL ===\n1 - Area do Aluno\n2 - Area do Professor\n3 - Secretaria\n0 - Sair\nEscolha: ");
        scanf("%d", &o); getchar();

        if (o == 1) {
            int opt, idAluno;
            do {
                printf("\n-- AREA DO ALUNO --\n1 - Cadastrar\n2 - Login\n0 - Voltar\nEscolha: ");
                scanf("%d", &opt); getchar();
                if (opt == 1) cadAluno();
                else if (opt == 2) {
                    idAluno = loginAluno(&idAluno);
                    if (idAluno != -1) menuAluno(idAluno);
                }
            } while (opt != 0);
        }

        else if (o == 2) {
            int opt;
            do {
                printf("\n-- AREA DO PROFESSOR --\n1 - Cadastrar\n2 - Login\n0 - Voltar\nEscolha: ");
                scanf("%d", &opt); getchar();
                if (opt == 1) cadProf();
                else if (opt == 2) {
                    int ok = loginProf();
                    if (ok != -1) menuProfessor();
                }
            } while (opt != 0);
        }

        else if (o == 3) menuSecretaria();

    } while (o != 0);
}

/* --- main --- */
int main() {
    inicializarBanco();
    menu();
    sqlite3_close(db);
    return 0;
}
