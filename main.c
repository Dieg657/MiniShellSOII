#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


/*
 * Alunos: Diego dos Santos Soares
 *         Welligton Stival
 *
 *
 */

#define max 100
int exec=0;
int back=0;
struct sigaction act;
int entrada_de_arquivo=0;
int saida_de_arquivo=0;
char nome_arquivo[max];
int pos_nome_arquivo=0;

int aux(char *** argumentos){
   int fda[2];
   pipe(fda);
   switch(fork()) {

           case 0:
               close(1);
               dup(fda[1]);
               close(fda[1]);
               close(fda[0]);
               execvp(argumentos[0][0],argumentos[0]);
               return 1;

           default:
               close(0);
               dup(fda[0]);
               close(fda[0]);
               close(fda[1]);
               execvp(argumentos[1][0],argumentos[0]);
               return 0;
   }
}

void clean(char * params, int fim);

int isolaArgumentos(char ** params, char * cadeia);

void trataSignal(int sig);

int main(){
    pid_t pid; //variavel para guardar id de processo
    signal(SIGINT,&trataSignal);

    for(;;) //Loop de Iteração do Shell
    {
           char * linha= malloc(sizeof(char)*100);
           char ** argumentos=malloc(sizeof(char*)*10);
           char *** lista_de_argumentos=malloc(sizeof(char**)*2);
           int lista_de_argumentos_atual=0;

           for(int j=0; j<5;j++)
           {
               lista_de_argumentos[j]=malloc(sizeof(char*)*10);
               for(int i=0; i<10; i++)
               {
                   argumentos[i]=malloc(sizeof(char)*15);
                   lista_de_argumentos[j][i]=malloc(sizeof(char)*15);
               }
           }

           clean(linha,100);
           printf("[MINISHELL]$: ");
           //Efetua a leitura de dados do usuário!
           scanf(" %[^\n]s", linha);

           setbuf(stdin, NULL);

           int debug=isolaArgumentos(lista_de_argumentos[lista_de_argumentos_atual],linha);
           while(debug==2)
           {
               lista_de_argumentos_atual++;
               debug=isolaArgumentos(lista_de_argumentos[lista_de_argumentos_atual],linha);
           }
           if(debug==1)
           {
                     if(strcmp(argumentos[0],"sair")==0 || strcmp(argumentos[0],"exit")==0)
                     {
                         break;
                     }
                     else if(strcmp(argumentos[0],"cd")==0)
                     {
                            chdir(argumentos[1]);
                     }
                     else if(exec==1)
                     {
                           goto SALTO;
                     }
                   else{
                       pid=fork();
                       if(pid==0)
                       {
                           SALTO:
                           if(saida_de_arquivo==1)
                           {
                               int fd;
                               fd = open(nome_arquivo,O_RDWR|O_CREAT|O_TRUNC,0666);
                               close(1);
                               dup(fd);
                           }
                           else if(entrada_de_arquivo==1)
                           {
                               int fd;
                               fd = open(nome_arquivo,0);
                               close(0);
                               dup(fd);
                           }

                           if(lista_de_argumentos_atual==0)
                           {
                               if(execvp(lista_de_argumentos[0][0],lista_de_argumentos[0])==-1){
                                   printf("Comando innvalido!\n");
                                   exit(0);
                               }
                           }
                           else{
                               int retorno=aux(lista_de_argumentos);
                               if(retorno==1)return 0;

                           }
                        }else if(back==0){
                           saida_de_arquivo=0;
                           clean(nome_arquivo,max);
                           pos_nome_arquivo=0;
                           entrada_de_arquivo=0;
                           waitpid(pid,0,0);
                       }
                    }
           }
           else printf("quantidade de parametros invalidos");
    }
    printf("Estou finalizando");
}


void trataSignal(int sig) {
    for(;;)
      {
          printf("Você deseja realmente sair: s-sim n-nao\n");
          setbuf(stdin, NULL);
          char aux=getchar();
          if(aux=='S' || aux=='s')
          {
              exit(0);
          }
          if(aux=='N' || aux=='n')
          {
              break;
          }
      }
      setbuf(stdin, NULL);
      printf("# ");
}

int isolaArgumentos(char ** params, char * cadeia)    //Função para separar varios argumentos
{                                           //na mesma linha onde encontrar espaços
    int v=0;
       for(int i=0; i<9;i++)clean(params[i],25);   //chama funcao para limpar lixos na matriz de
       int i,j;                                    //argumentos
       for(i=0, j=0 ; i<max; i++, j++) {         ///interage ate o final da linha de comando
             if(cadeia[i]==0)
           {
               j--;
           }
           else if(cadeia[i]==' '){                  ///se durante a varredura da linha do comando
                                             ///ele encontrar ' ', executa...
                j=-1;                          //
                v++;                           //
                if(v>=10)                      //suporta somente 10 argumentos
                {
                    return 0;                  //se passar, para de ler codigos
                }
                if(strcmp(params[0],"exec")==0)//se o primeiro argumento for == "exec"
                {                              //ativa uma flag global que usamos para
                    clean(params[0],10);       //chamar o codigo e depois morrer
                   exec=1;
                   v--;
                }
            }
            else{
               if(cadeia[i]=='<')
               {
                   entrada_de_arquivo=1;
                   i++;
                   v--;
                 }
               else if(cadeia[i]=='>')
               {
                   saida_de_arquivo=1;
                   i++;
                   v--;
               }
               else if(cadeia[i]=='|')
               {
                   cadeia[i]=0;
                   cadeia[i+1]=0;
                       while(v<10)
                       {
                           params[v]=0;
                           v++;
                       }
                   return 2;
               }
               else if(cadeia[i]=='\0')break;
               else if(entrada_de_arquivo==1 || saida_de_arquivo==1)
               {
                   nome_arquivo[pos_nome_arquivo]=cadeia[i];
                   pos_nome_arquivo++;
               }
               else
               {
                    params[v][j]=cadeia[i];
               }
            }
            cadeia[i]=0;
       }
       if(strcmp(params[v],"&")==0)          //se no ultimo argumento
       {                                     //encontrar  '&' ativa uma flag
           back=1;                           //para desativar o wait do proscesso filho
           v--;
       }
       v++;
       while(v<10)
       {
           params[v]=0;                     //adiciona 0 na posicao do argumento na matriz
           v++;
       }
       return 1; //Isolou os parametros corretamente
}

void clean(char * params, int fim)  //função para limpar string de argumentos
{
    for(int i=0; i<fim; i++)
    {
        params[i]=0;
    }
}
