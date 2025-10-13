#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

Aluno alunos[MAX_ALUNOS]; int qtdAlunos = 0;
Professor professores[MAX_PROFESSORES]; int qtdProfessores = 0, proxIdProf = 1;
Turma turmas[MAX_TURMAS]; int qtdTurmas = 0, proxIdTurma = 1;

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
    fclose(f);
}

void carregar(){
    FILE*f=fopen("dados.bin","rb");if(!f)return;
    fread(&qtdAlunos,sizeof(int),1,f);fread(alunos,sizeof(Aluno),qtdAlunos,f);
    fread(&qtdProfessores,sizeof(int),1,f);fread(professores,sizeof(Professor),qtdProfessores,f);
    fread(&qtdTurmas,sizeof(int),1,f);fread(turmas,sizeof(Turma),qtdTurmas,f);
    fread(&proxIdProf,sizeof(int),1,f);fread(&proxIdTurma,sizeof(int),1,f);
    fclose(f);
}

void cadAluno(){
    if(qtdAlunos>=MAX_ALUNOS)return;
    Aluno a;char raw[MAX_EMAIL];
    printf("Nome: ");linha(a.nome,MAX_NOME);
    printf("Email: ");linha(raw,MAX_EMAIL);lower(raw,a.email,MAX_EMAIL);
    if(existeEmail(a.email)){printf("Email ja existe.\n");return;}
    printf("Senha: ");linha(a.senha,MAX_SENHA);
    alunos[qtdAlunos++]=a;printf("Aluno cadastrado.\n");
}

int loginAluno(char out[MAX_EMAIL]){
    char raw[MAX_EMAIL],e[MAX_EMAIL],s[MAX_SENHA];
    printf("Email: ");linha(raw,MAX_EMAIL);lower(raw,e,MAX_EMAIL);
    int i=idxAluno(e);if(i==-1){printf("Nao encontrado.\n");return -1;}
    printf("Senha: ");linha(s,MAX_SENHA);
    if(!strcmp(alunos[i].senha,s)){strcpy(out,e);printf("Bem-vindo %s!\n",alunos[i].nome);return i;}
    printf("Senha incorreta.\n");return -1;
}

void menuAluno(const char *e){
    int o;
    do{
        printf("\n-- AREA DO ALUNO --\n1 - Minhas Turmas\n0 - Voltar\nEscolha: ");
        if(scanf("%d",&o)!=1){while(getchar()!='\n');o=-1;}getchar();
        if(o==1){
            int f=0;
            for(int i=0;i<qtdTurmas;++i)
                if(alunoNaTurma(&turmas[i],e)){printf("Turma %d | %s\n",turmas[i].id,turmas[i].nome);f=1;}
            if(!f)printf("Nenhuma turma encontrada.\n");
        }
    }while(o!=0);
}

void cadProf(){
    if(qtdProfessores>=MAX_PROFESSORES)return;
    Professor p;char raw[MAX_EMAIL];
    p.id=proxIdProf++;
    printf("Nome: ");linha(p.nome,MAX_NOME);
    printf("Email: ");linha(raw,MAX_EMAIL);lower(raw,p.email,MAX_EMAIL);
    if(existeEmail(p.email)){printf("Email ja existe.\n");return;}
    printf("Senha: ");linha(p.senha,MAX_SENHA);
    professores[qtdProfessores++]=p;printf("Professor cadastrado.\n");
}

int loginProf(){
    char raw[MAX_EMAIL],e[MAX_EMAIL],s[MAX_SENHA];
    printf("Email: ");linha(raw,MAX_EMAIL);lower(raw,e,MAX_EMAIL);
    int i=idxProf(e);if(i==-1){printf("Nao encontrado.\n");return -1;}
    printf("Senha: ");linha(s,MAX_SENHA);
    if(!strcmp(professores[i].senha,s)){printf("Bem-vindo %s!\n",professores[i].nome);return professores[i].id;}
    printf("Senha incorreta.\n");return -1;
}

void criarTurma(){
    if(qtdTurmas>=MAX_TURMAS)return;
    Turma t;t.id=proxIdTurma++;t.qtd=0;
    printf("Nome da turma: ");linha(t.nome,60);
    turmas[qtdTurmas++]=t;
    printf("Turma criada (ID %d)\n",t.id);
}

void listarTurmas(){
    if(!qtdTurmas){printf("Nenhuma turma.\n");return;}
    for(int i=0;i<qtdTurmas;++i)
        printf("ID: %d | %s | %d alunos\n",turmas[i].id,turmas[i].nome,turmas[i].qtd);
}

void listarAlunos(){
    if(!qtdAlunos){printf("Nenhum aluno.\n");return;}
    for(int i=0;i<qtdAlunos;++i)
        printf("%s | %s\n",alunos[i].email,alunos[i].nome);
}

void addAlunoTurma(){
    listarTurmas();
    int id;printf("ID da turma: ");
    if(scanf("%d",&id)!=1){while(getchar()!='\n');return;}getchar();
    int it=idxTurma(id);if(it==-1){printf("Turma nao encontrada.\n");return;}
    Turma *t=&turmas[it];
    listarAlunos();
    char raw[MAX_EMAIL],e[MAX_EMAIL];
    printf("Email do aluno: ");linha(raw,MAX_EMAIL);lower(raw,e,MAX_EMAIL);
    if(idxAluno(e)==-1){printf("Nao existe.\n");return;}
    if(alunoNaTurma(t,e)){printf("Ja esta na turma.\n");return;}
    strcpy(t->alunos[t->qtd++],e);
    printf("Adicionado.\n");
}

void menuAdm(){
    int o;
    do{
        printf("\n-- ADMINISTRACAO --\n1 - Criar Turma\n2 - Listar Turmas\n3 - Adicionar Aluno em Turma\n4 - Listar Alunos\n0 - Voltar\nEscolha: ");
        if(scanf("%d",&o)!=1){while(getchar()!='\n');o=-1;}getchar();
        if(o==1)criarTurma();
        else if(o==2)listarTurmas();
        else if(o==3)addAlunoTurma();
        else if(o==4)listarAlunos();
    }while(o!=0);
}

void menu(){
    int o;
    do{
        printf("\n=== MENU PRINCIPAL ===\n1 - Area do Aluno\n2 - Area do Professor\n3 - Administracao\n0 - Sair\nEscolha: ");
        if(scanf("%d",&o)!=1){while(getchar()!='\n');o=-1;}getchar();
        if(o==1){
            int op;
            do{
                printf("\n-- AREA DO ALUNO --\n1 - Cadastrar\n2 - Login\n0 - Voltar\nEscolha: ");
                if(scanf("%d",&op)!=1){while(getchar()!='\n');op=-1;}getchar();
                if(op==1)cadAluno();
                else if(op==2){
                    char e[MAX_EMAIL];
                    if(loginAluno(e)!=-1)menuAluno(e);
                }
            }while(op!=0);
        }else if(o==2){
            int op;
            do{
                printf("\n-- AREA DO PROFESSOR --\n1 - Cadastrar\n2 - Login\n0 - Voltar\nEscolha: ");
                if(scanf("%d",&op)!=1){while(getchar()!='\n');op=-1;}getchar();
                if(op==1)cadProf();
                else if(op==2)loginProf();
            }while(op!=0);
        }else if(o==3)menuAdm();
        else if(o==0){salvar();printf("Saindo...\n");}
    }while(o!=0);
}

int main(){
    carregar();
    menu();
    return 0;
}


