#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ALUNOS 300
#define MAX_PROFESSORES 100
#define MAX_TURMAS 100
#define MAX_NOME 100
#define MAX_EMAIL 100
#define MAX_SENHA 32

typedef struct {char email[MAX_EMAIL], nome[MAX_NOME], senha[MAX_SENHA];} Aluno;
typedef struct {char email[MAX_EMAIL], nome[MAX_NOME], senha[MAX_SENHA]; int id;} Professor;
typedef struct {int id, qtd; char nome[60], alunos[MAX_ALUNOS][MAX_EMAIL];} Turma;

typedef struct { float prova1; float prova2; } Notas;

Aluno alunos[MAX_ALUNOS]; int qtdAlunos = 0;
Professor professores[MAX_PROFESSORES]; int qtdProfessores = 0, proxIdProf = 1;
Turma turmas[MAX_TURMAS]; int qtdTurmas = 0, proxIdTurma = 1;
Notas notas[MAX_ALUNOS][MAX_TURMAS];

const char *cursos[] = {
    "ENGENHARIA DE SOFTWARE AGIL",
    "PROGRAMACAO ESTRUTURADA EM C",
    "ANALISE E PROJETO DE SISTEMAS"
};

void linha(char *b, int s){if(fgets(b,s,stdin)){size_t n=strlen(b);if(n>0&&b[n-1]=='\n')b[n-1]='\0';}}
void lower(const char *src,char *dst,int sz){int i;for(i=0;i<sz-1&&src[i];++i)dst[i]=tolower((unsigned char)src[i]);dst[i]='\0';}

int idxAluno(const char *e){for(int i=0;i<qtdAlunos;++i)if(!strcmp(alunos[i].email,e))return i;return -1;}
int idxProf(const char *e){for(int i=0;i<qtdProfessores;++i)if(!strcmp(professores[i].email,e))return i;return -1;}
int idxTurma(int id){for(int i=0;i<qtdTurmas;++i)if(turmas[i].id==id)return i;return -1;}
int alunoNaTurma(const Turma *t,const char *e){for(int i=0;i<t->qtd;++i)if(!strcmp(t->alunos[i],e))return 1;return 0;}
int existeEmail(const char *e){return idxAluno(e)!=-1||idxProf(e)!=-1;}

void salvar(){
    FILE*f=fopen("dados.bin","wb");if(!f)return;
    fwrite(&qtdAlunos,sizeof(int),1,f);fwrite(alunos,sizeof(Aluno),qtdAlunos,f);
    fwrite(&qtdProfessores,sizeof(int),1,f);fwrite(professores,sizeof(Professor),qtdProfessores,f);
    fwrite(&qtdTurmas,sizeof(int),1,f);fwrite(turmas,sizeof(Turma),qtdTurmas,f);
    fwrite(&proxIdProf,sizeof(int),1,f);fwrite(&proxIdTurma,sizeof(int),1,f);
    fwrite(&notas,sizeof(Notas),MAX_ALUNOS*MAX_TURMAS,f);
    fclose(f);
}

void carregar(){
    FILE*f=fopen("dados.bin","rb");if(!f)return;
    fread(&qtdAlunos,sizeof(int),1,f);fread(alunos,sizeof(Aluno),qtdAlunos,f);
    fread(&qtdProfessores,sizeof(int),1,f);fread(professores,sizeof(Professor),qtdProfessores,f);
    fread(&qtdTurmas,sizeof(int),1,f);fread(turmas,sizeof(Turma),qtdTurmas,f);
    fread(&proxIdProf,sizeof(int),1,f);fread(&proxIdTurma,sizeof(int),1,f);
    fread(&notas,sizeof(Notas),MAX_ALUNOS*MAX_TURMAS,f);
    fclose(f);
}

void cadAluno(){
    if(qtdAlunos>=MAX_ALUNOS)return;
    Aluno a; char raw[MAX_EMAIL];

    do{ printf("Nome (min 4 caracteres): "); linha(a.nome, MAX_NOME);
        if(strlen(a.nome)<4) printf("Nome muito curto!\n");
    }while(strlen(a.nome)<4);

    do{ printf("Email (min 4 caracteres): "); linha(raw, MAX_EMAIL); lower(raw, a.email, MAX_EMAIL);
        if(strlen(a.email)<4) printf("Email muito curto!\n");
        else if(existeEmail(a.email)){printf("Email ja existe.\n"); return;}
    }while(strlen(a.email)<4);

    do{ printf("Senha (min 4 caracteres): "); linha(a.senha, MAX_SENHA);
        if(strlen(a.senha)<4) printf("Senha muito curta!\n");
    }while(strlen(a.senha)<4);

    alunos[qtdAlunos++]=a; printf("Aluno cadastrado.\n");
}

void cadProf(){
    if(qtdProfessores>=MAX_PROFESSORES)return;
    Professor p; char raw[MAX_EMAIL];
    p.id = proxIdProf++;

    do{ printf("Nome (min 4 caracteres): "); linha(p.nome, MAX_NOME);
        if(strlen(p.nome)<4) printf("Nome muito curto!\n");
    }while(strlen(p.nome)<4);

    do{ printf("Email (min 4 caracteres): "); linha(raw, MAX_EMAIL); lower(raw, p.email, MAX_EMAIL);
        if(strlen(p.email)<4) printf("Email muito curto!\n");
        else if(existeEmail(p.email)){printf("Email ja existe.\n"); return;}
    }while(strlen(p.email)<4);

    do{ printf("Senha (min 4 caracteres): "); linha(p.senha, MAX_SENHA);
        if(strlen(p.senha)<4) printf("Senha muito curta!\n");
    }while(strlen(p.senha)<4);

    professores[qtdProfessores++] = p; printf("Professor cadastrado.\n");
}

int loginAluno(char out[MAX_EMAIL]){
    char raw[MAX_EMAIL],e[MAX_EMAIL],s[MAX_SENHA];
    printf("Email: "); linha(raw,MAX_EMAIL); lower(raw,e,MAX_EMAIL);
    int i=idxAluno(e); if(i==-1){printf("Nao encontrado.\n"); return -1;}
    printf("Senha: "); linha(s,MAX_SENHA);
    if(!strcmp(alunos[i].senha,s)){strcpy(out,e); printf("Bem-vindo %s!\n",alunos[i].nome); return i;}
    printf("Senha incorreta.\n"); return -1;
}

int loginProf(){
    char raw[MAX_EMAIL],e[MAX_EMAIL],s[MAX_SENHA];
    printf("Email: "); linha(raw,MAX_EMAIL); lower(raw,e,MAX_EMAIL);
    int i=idxProf(e); if(i==-1){printf("Nao encontrado.\n"); return -1;}
    printf("Senha: "); linha(s,MAX_SENHA);
    if(!strcmp(professores[i].senha,s)){printf("Bem-vindo %s!\n",professores[i].nome); return professores[i].id;}
    printf("Senha incorreta.\n"); return -1;
}

void criarTurma(){
    if(qtdTurmas>=MAX_TURMAS) return;
    Turma t; t.id = proxIdTurma++; t.qtd = 0;

    printf("Escolha o curso:\n");
    for(int i=0;i<3;i++) printf("%d - %s\n",i+1,cursos[i]);
    int op; scanf("%d",&op); getchar();
    if(op<1||op>3){ printf("Opcao invalida.\n"); return; }
    strcpy(t.nome, cursos[op-1]);

    turmas[qtdTurmas++] = t;
    printf("Turma criada (ID %d) - %s\n", t.id, t.nome);
}

void listarTurmas(){
    if(!qtdTurmas){printf("Nenhuma turma.\n");return;}
    for(int i=0;i<qtdTurmas;i++)
        printf("ID: %d | %s | %d alunos\n", turmas[i].id, turmas[i].nome, turmas[i].qtd);
}

void listarAlunos(){
    if(!qtdAlunos){printf("Nenhum aluno.\n");return;}
    for(int i=0;i<qtdAlunos;i++)
        printf("%s | %s\n", alunos[i].email, alunos[i].nome);
}

void addAlunoTurma(){
    listarTurmas();
    int id; printf("ID da turma: ");
    if(scanf("%d",&id)!=1){while(getchar()!='\n');return;} getchar();
    int it = idxTurma(id); if(it==-1){printf("Turma nao encontrada.\n"); return;}
    Turma *t = &turmas[it];

    listarAlunos();
    char raw[MAX_EMAIL],e[MAX_EMAIL];
    printf("Email do aluno: "); linha(raw, MAX_EMAIL); lower(raw,e,MAX_EMAIL);
    int ia = idxAluno(e); if(ia==-1){printf("Aluno nao encontrado.\n"); return;}
    if(alunoNaTurma(t,e)){printf("Aluno ja esta na turma.\n"); return;}

    strcpy(t->alunos[t->qtd++], e);
    notas[ia][it].prova1 = 0; notas[ia][it].prova2 = 0; // inicia notas
    printf("Aluno adicionado.\n");
}

void lancarNotas(){
    listarTurmas();
    int id; printf("ID da turma: "); if(scanf("%d",&id)!=1){while(getchar()!='\n');return;} getchar();
    int it = idxTurma(id); if(it==-1){printf("Turma nao encontrada.\n"); return;}
    Turma *t = &turmas[it];

    listarAlunos();
    char raw[MAX_EMAIL], e[MAX_EMAIL]; printf("Email do aluno: "); linha(raw,MAX_EMAIL); lower(raw,e,MAX_EMAIL);
    int ia = idxAluno(e); if(ia==-1){printf("Aluno nao encontrado.\n"); return;}
    if(!alunoNaTurma(t,e)){printf("Aluno nao esta nesta turma.\n"); return;}

    float p1,p2; printf("Nota Prova 1: "); scanf("%f",&p1); getchar();
    printf("Nota Prova 2: "); scanf("%f",&p2); getchar();
    notas[ia][it].prova1 = p1; notas[ia][it].prova2 = p2;
    printf("Notas lançadas com sucesso.\n");
}

void verBoletim(int idxAlunoLogado){
    int f = 0;
    for(int i=0;i<qtdTurmas;i++){
        if(alunoNaTurma(&turmas[i], alunos[idxAlunoLogado].email)){
            f=1;
            float media = (notas[idxAlunoLogado][i].prova1 + notas[idxAlunoLogado][i].prova2)/2.0;
            printf("\nTurma: %s\nProva1: %.2f\nProva2: %.2f\nMedia: %.2f\nStatus: %s\n",
                   turmas[i].nome,
                   notas[idxAlunoLogado][i].prova1,
                   notas[idxAlunoLogado][i].prova2,
                   media,
                   media>=7.0 ? "Aprovado" : "Reprovado");
        }
    }
    if(!f) printf("Nenhuma turma encontrada.\n");
}

void menuAluno(const char *e){
    int idx = idxAluno(e), o;
    do{
        printf("\n-- AREA DO ALUNO --\n1 - Minhas Turmas\n2 - Ver Boletim\n0 - Voltar\nEscolha: ");
        if(scanf("%d",&o)!=1){while(getchar()!='\n'); o=-1;} getchar();
        if(o==1){
            int f=0;
            for(int i=0;i<qtdTurmas;i++)
                if(alunoNaTurma(&turmas[i],e)){printf("Turma %d | %s\n", turmas[i].id, turmas[i].nome); f=1;}
            if(!f) printf("Nenhuma turma encontrada.\n");
        }
        else if(o==2) verBoletim(idx);
    }while(o!=0);
}

void menuProf(int idProf){
    int o;
    do{
        printf("\n-- AREA DO PROFESSOR --\n1 - Lancar Notas\n0 - Voltar\nEscolha: ");
        if(scanf("%d",&o)!=1){while(getchar()!='\n'); o=-1;} getchar();
        if(o==1) lancarNotas();
    }while(o!=0);
}

void menuAdm(){
    int o;
    do{
        printf("\n-- ADMINISTRACAO --\n1 - Criar Turma\n2 - Listar Turmas\n3 - Adicionar Aluno em Turma\n4 - Listar Alunos\n0 - Voltar\nEscolha: ");
        if(scanf("%d",&o)!=1){while(getchar()!='\n'); o=-1;} getchar();
        if(o==1) criarTurma();
        else if(o==2) listarTurmas();
        else if(o==3) addAlunoTurma();
        else if(o==4) listarAlunos();
    }while(o!=0);
}

void menu(){
    int o;
    do{
        printf("\n=== MENU PRINCIPAL ===\n1 - Area do Aluno\n2 - Area do Professor\n3 - Administracao\n0 - Sair\nEscolha: ");
        if(scanf("%d",&o)!=1){while(getchar()!='\n'); o=-1;} getchar();
        if(o==1){
            int op;
            do{
                printf("\n-- AREA DO ALUNO --\n1 - Cadastrar\n2 - Login\n0 - Voltar\nEscolha: ");
                if(scanf("%d",&op)!=1){while(getchar()!='\n'); op=-1;} getchar();
                if(op==1) cadAluno();
                else if(op==2){ char e[MAX_EMAIL]; if(loginAluno(e)!=-1) menuAluno(e);}
            }while(op!=0);
        }
        else if(o==2){
            int op;
            do{
                printf("\n-- AREA DO PROFESSOR --\n1 - Cadastrar\n2 - Login\n0 - Voltar\nEscolha: ");
                if(scanf("%d",&op)!=1){while(getchar()!='\n'); op=-1;} getchar();
                if(op==1) cadProf();
                else if(op==2){ int id = loginProf(); if(id!=-1) menuProf(id);}
            }while(op!=0);
        }
        else if(o==3) menuAdm();
        else if(o==0){ salvar(); printf("Saindo...\n");}
    }while(o!=0);
}

int main(){
    carregar();
    menu();
    return 0;
}
